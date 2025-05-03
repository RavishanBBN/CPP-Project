#include "../include/ThreadManager.h"
#include <stdexcept>

ThreadManager::ThreadManager(size_t n): count(n)
{
    if(n==0) throw std::invalid_argument("Thread count must be >0");
}

ThreadManager::~ThreadManager(){ stop(); }

void ThreadManager::start()
{
    if(running.load()) return;
    running.store(true);
    for(size_t i=0;i<count;++i)
        workers.emplace_back(&ThreadManager::workerLoop,this);
}

void ThreadManager::stop()
{
    if(!running.load()) return;
    {
        std::lock_guard lk(mtx);
        running.store(false);
    }
    cv.notify_all();
    for(auto& t:workers) if(t.joinable()) t.join();
    workers.clear();
}

bool ThreadManager::pop(std::function<void()>& job)
{
    std::unique_lock lk(mtx);
    cv.wait(lk,[&]{ return !running.load() || !q.empty(); });
    if(!running.load() && q.empty()) return false;
    job=std::move(q.front()); q.pop();
    ++active;
    return true;
}

void ThreadManager::workerLoop()
{
    std::function<void()> job;
    while(pop(job))
    {
        job();
        --active;
        if(q.empty() && active.load()==0) doneCv.notify_all();
    }
}

void ThreadManager::addTask(const std::function<void()>& job)
{
    {
        std::lock_guard lk(mtx);
        q.push(job);
    }
    cv.notify_one();
}

void ThreadManager::waitForCompletion()
{
    std::unique_lock lk(mtx);
    doneCv.wait(lk,[&]{ return q.empty() && active.load()==0; });
}

size_t ThreadManager::getTaskCount() const
{
    std::lock_guard lk(mtx);
    return q.size();
}

void ThreadManager::setNumThreads(size_t n)
{
    stop(); count=n; start();
}
