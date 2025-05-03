#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadManager {
public:
    explicit ThreadManager(size_t n = std::thread::hardware_concurrency());
    ~ThreadManager();

    void start();
    void stop();
    void addTask(const std::function<void()>& job);
    void waitForCompletion();

    // info
    bool   isRunning()            const { return running.load(); }
    size_t getNumThreads()        const { return count; }
    size_t getTaskCount()         const;
    size_t getActiveThreadCount() const { return active.load(); }

    void setNumThreads(size_t n);

    ThreadManager(const ThreadManager&)            = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

private:
    void workerLoop();
    bool pop(std::function<void()>& job);

    size_t count;
    std::vector<std::thread> workers;

    std::queue<std::function<void()>> q;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::condition_variable doneCv;

    std::atomic<bool>    running{false};
    std::atomic<size_t>  active{0};
};
