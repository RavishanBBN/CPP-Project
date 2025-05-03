#include "../include/ThreadManager.h"
#include <stdexcept>

// ------------------------------------------------------------------ ctor / dtor
ThreadManager::ThreadManager(size_t n) : threadCount(n)
{
    if (n == 0)
        throw std::invalid_argument("ThreadManager: numThreads must be > 0");
}

ThreadManager::~ThreadManager() { stop(); }

// ------------------------------------------------------------------ start / stop
void ThreadManager::start()
{
    if (running.load()) return;            // already running
    running.store(true);
    workers.reserve(threadCount);
    for (size_t i = 0; i < threadCount; ++i)
        workers.emplace_back(&ThreadManager::workerLoop, this);
}

void ThreadManager::stop()
{
    if (!running.load()) return;
    {
        std::lock_guard lk(jobsMtx);
        running.store(false);
    }
    jobsCv.notify_all();
    for (auto& t : workers)
        if (t.joinable()) t.join();
    workers.clear();
}

// ------------------------------------------------------------------ resize
void ThreadManager::setNumThreads(size_t n)
{
    if (n == threadCount) return;
    stop();
    threadCount = n;
    start();
}

// ------------------------------------------------------------------ enqueue
void ThreadManager::addTask(const std::function<void()>& job)
{
    {
        std::lock_guard lk(jobsMtx);
        jobs.push(job);
    }
    jobsCv.notify_one();
}

// ------------------------------------------------------------------ pop helper
bool ThreadManager::popTask(std::function<void()>& job)
{
    std::unique_lock lk(jobsMtx);
    jobsCv.wait(lk, [&]{ return !running.load() || !jobs.empty(); });
    if (!running.load() && jobs.empty()) return false;
    job = std::move(jobs.front());
    jobs.pop();
    ++active;
    return true;
}

// ------------------------------------------------------------------ worker loop
void ThreadManager::workerLoop()
{
    std::function<void()> job;
    while (true)
    {
        if (!popTask(job)) break;          // pool shutting down
        job();                             // run
        --active;

        // wake anyone waiting for all work to finish
        if (jobs.empty() && active.load() == 0)
            doneCv.notify_all();
    }
}

// ------------------------------------------------------------------ wait
void ThreadManager::waitForCompletion()
{
    std::unique_lock lk(jobsMtx);
    doneCv.wait(lk, [&]{ return jobs.empty() && active.load() == 0; });
}

// ------------------------------------------------------------------ queries
size_t ThreadManager::getTaskCount()        const { std::lock_guard lk(jobsMtx); return jobs.size(); }
size_t ThreadManager::getActiveThreadCount()const { return active.load(); }
