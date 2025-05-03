#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <queue>

class ThreadManager {
public:
    explicit ThreadManager(size_t numThreads);
    ~ThreadManager();

    void start();
    void stop();
    void waitForCompletion();
    void addTask(std::function<void()> task);

    size_t getTaskCount() const;
    size_t getNumThreads() const;
    void   setNumThreads(size_t n);
    size_t getActiveThreadCount() const;
    bool   isRunning() const;

private:
    void workerThread();

    mutable std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;

    std::vector<std::thread> threads;
    std::atomic<bool> running{false};
    std::atomic<size_t> activeCount{0};
    size_t threadCount;
};
