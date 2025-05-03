#include "ThreadManager.h"
#include <stdexcept>

ThreadManager::ThreadManager(size_t n)
  : threadCount(n)
{
    if (threadCount==0) throw std::invalid_argument("Thread count>0");
}

ThreadManager::~ThreadManager() { stop(); }

void ThreadManager::start() {
    running=true;
    for(size_t i=0;i<threadCount;i++)
        threads.emplace_back(&ThreadManager::workerThread,this);
}

void ThreadManager::stop() {
    running=false;
    cv.notify_all();
    for(auto &t:threads)
        if(t.joinable()) t.join();
    threads.clear();
}

void ThreadManager::addTask(std::function<void()> task) {
    { std::lock_guard l(queueMutex); tasks.push(std::move(task)); }
    cv.notify_one();
}

size_t ThreadManager::getTaskCount() const {
    std::lock_guard l(queueMutex); return tasks.size();
}

bool ThreadManager::isRunning() const { return running; }
size_t ThreadManager::getNumThreads() const { return threadCount; }

void ThreadManager::setNumThreads(size_t n) {
    if(n==0) throw std::invalid_argument("count>0");
    bool was=running; stop(); threadCount=n;
    if(was) start();
}

size_t ThreadManager::getActiveThreadCount() const { return activeCount; }

void ThreadManager::waitForCompletion() {
    std::unique_lock l(queueMutex);
    cv.wait(l, [&]{ return tasks.empty() && activeCount==0; });
}

void ThreadManager::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock l(queueMutex);
            cv.wait(l, [&]{ return !running || !tasks.empty(); });
            if(!running && tasks.empty()) return;
            task = std::move(tasks.front()); tasks.pop();
            activeCount++;
        }
        try { task(); } catch(...) {}
        activeCount--;
        cv.notify_all();
    }
}
