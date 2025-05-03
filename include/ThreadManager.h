#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <queue>

/// Simple thread‑pool that executes std::function<void()> tasks.
class ThreadManager {
public:
    explicit ThreadManager(size_t numThreads);
    ~ThreadManager();

    // --- lifecycle -------------------------------------------------
    void start();             ///< spawn worker threads (idempotent)
    void stop();              ///< finish queued tasks, join threads
    void waitForCompletion(); ///< block until queue empty & workers idle

    // --- enqueue ---------------------------------------------------
    void addTask(const std::function<void()>& task);

    // --- introspection --------------------------------------------
    size_t getTaskCount()        const;
    size_t getActiveThreadCount()const;
    size_t getNumThreads()       const { return numThreads; }
    bool   isRunning()           const { return running.load(std::memory_order_acquire); }

    // Dynamically resize pool (stop + restart).
    void setNumThreads(size_t n);

    ThreadManager(const ThreadManager&)            = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

private:
    void workerThread(size_t id);              // function run by each worker

    bool popTask(std::function<void()>& task); // helper: get next task

    // --- data members ---------------------------------------------
    size_t                        numThreads;
    std::vector<std::thread>      threads;

    std::queue<std::function<void()>> taskQueue;
    mutable std::mutex            queueMtx;
    std::condition_variable       queueCv;

    std::condition_variable       doneCv;      // used by waitForCompletion
    std::atomic<bool>             running {false};
    std::atomic<size_t>           activeThreads {0};
};
