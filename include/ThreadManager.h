#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <queue>

/// Very small fixed‑size thread‑pool.
///  • addTask() puts a job in the queue
///  • waitForCompletion() blocks until queue empty + workers idle
class ThreadManager {
public:
    explicit ThreadManager(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadManager();

    void start();             ///< spawn workers (idempotent)
    void stop();              ///< finish queue, join threads
    void waitForCompletion(); ///< wait until queue empty & idle

    void addTask(const std::function<void()>& job);

    // --- runtime info ---------------------------------------------
    size_t getTaskCount()        const;
    size_t getActiveThreadCount()const;
    size_t getNumThreads()       const { return threadCount; }
    bool   isRunning()           const { return running.load(std::memory_order_acquire); }

    // Resize pool – stops current workers then restarts with new size
    void setNumThreads(size_t n);

    ThreadManager(const ThreadManager&)            = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

private:
    void workerLoop();                       // main loop for each worker
    bool popTask(std::function<void()>& job); // helper: wait & pop

    // --- data ------------------------------------------------------
    size_t                       threadCount;
    std::vector<std::thread>     workers;

    std::queue<std::function<void()>> jobs;
    mutable std::mutex           jobsMtx;
    std::condition_variable      jobsCv;

    std::condition_variable      doneCv;     // used by waitForCompletion
    std::atomic<bool>            running{false};
    std::atomic<size_t>          active{0};
};
