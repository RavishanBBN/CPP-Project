#include "../include/ThreadManager.h"
#include <stdexcept>

ThreadManager::ThreadManager(size_t n) : numThreads(n)
{
    if (n == 0)
        throw std::invalid_argument("ThreadManager: numThreads must be > 0");
}

ThreadManager::~ThreadManager()
{
    stop();
}

// ------------------------------------------------------------------
void ThreadManager::start()
{
    if (running.load()) return;        // already running
    running.store(true);
    threads.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i)
        threads.emplace_back(&ThreadManager::workerThread, this, i);
}

// ------------------------------------------------------------------
void ThreadManager::stop()
{
    if (!running.load()) return;
    {
        std::scoped_lock lock(queueMtx);
        running.store(false);
    }
    queueCv.notify_all();
    for (auto& t : threads)
        if (t.joinable()) t.join();
    threads.clear();
}

// ------------------------------------------------------------------
void ThreadManager::addTask(const std::function<void()>& task)
{
    {
        std::scoped_lock lock(queueMtx);
        taskQueue.push(task);
    }
    queueCv.notify_one();
}

// ------------------------------------------------------------------
bool ThreadManager::popTask(std::function<void()>& task)
{
    std::unique_lock lock(queueMtx);
    queueCv.wait(lock, [&]{ return !running.load() || !taskQueue.empty(); });
    if (!running.load() && taskQueue.empty())
        return false;                         // shutting down
    task = std::move(taskQueue.front());
    taskQueue.pop();
    ++activeThreads;
    return true;
}

// ------------------------------------------------------------------
void ThreadManager::workerThread(size_t /*id*/)
{
    std::function<void()> job;
    while (true)
    {
        if (!popTask(job)) break;             // shutdown
        job();                                // run task
        --activeThreads;

        // notify waitForCompletion when queue empty & no active workers
        if (taskQueue.empty() && activeThreads.load() == 0)
            doneCv.notify_all();
    }
}

// ------------------------------------------------------------------
void ThreadManager::waitForCompletion()
{
    std::unique_lock lock(queueMtx);
    doneCv.wait(lock, [&]{ return taskQueue.empty() && activeThreads.load() == 0; });
}

// ------------------------------------------------------------------
size_t ThreadManager::getTaskCount() const
{
    std::scoped_lock lock(queueMtx);
    return taskQueue.size();
}

size_t ThreadManager::getActiveThreadCount() const { return activeThreads.load(); }

// ------------------------------------------------------------------
void ThreadManager::setNumThreads(size_t n)
{
    if (n == numThreads) return;
    stop();
    numThreads = n;
    start();
}
