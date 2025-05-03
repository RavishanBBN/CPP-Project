// src/ThreadManager.cpp
#include "ThreadManager.h"
#include <stdexcept>

ThreadManager::ThreadManager(size_t numThreads)
  : threadCount_(numThreads)
{
    if (threadCount_ == 0)
        throw std::invalid_argument("Thread count must be > 0");
}

ThreadManager::~ThreadManager() {
    stop();
}

void ThreadManager::start() {
    running_ = true;
    for (size_t i = 0; i < threadCount_; ++i) {
        threads_.emplace_back(&ThreadManager::workerThread, this);
    }
}

void ThreadManager::stop() {
    running_ = false;
    taskCv_.notify_all();
    for (auto& t : threads_) {
        if (t.joinable()) t.join();
    }
    threads_.clear();
}

void ThreadManager::addTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        taskQueue_.push(std::move(task));
    }
    taskCv_.notify_one();
}

size_t ThreadManager::getTaskCount() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return taskQueue_.size();
}

bool ThreadManager::isRunning() const {
    return running_.load();
}

size_t ThreadManager::getNumThreads() const {
    return threadCount_;
}

void ThreadManager::setNumThreads(size_t numThreads) {
    if (numThreads == 0)
        throw std::invalid_argument("Thread count must be > 0");
    bool wasRunning = running_.load();
    stop();
    threadCount_ = numThreads;
    if (wasRunning) start();
}

size_t ThreadManager::getActiveThreadCount() const {
    return activeCount_.load();
}

void ThreadManager::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    taskCv_.wait(lock, [this]() {
        return taskQueue_.empty() && (activeCount_.load() == 0);
    });
}

void ThreadManager::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            taskCv_.wait(lock, [this]() {
                return !running_ || !taskQueue_.empty();
            });
            if (!running_ && taskQueue_.empty())
                return;
            task = std::move(taskQueue_.front());
            taskQueue_.pop();
            activeCount_++;
        }

        try {
            task();
        } catch (...) {
            // swallow exceptions
        }

        activeCount_--;
        taskCv_.notify_all();
    }
}
