#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <queue>

/// Simple fixed‑size thread pool.
/// ‑ addTask() enqueues a void() job.
/// ‑ waitForCompletion() blocks until the queue empties.
class ThreadManager {
public:
    explicit ThreadManager(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadManager();

    /// Launch worker threads (idempotent).
    void start();
    /// Gracefully stop workers and join threads.
    void stop();

    /// Block until every queued job has finished.
    void waitForCompletion();

    /// Add a new job to the queue (thread‑safe).
    void addTask(const std::function<void()>& task);

    /// --- Introspection helpers ---
    size_t getTaskCount()       const;   // items waiting in queue
    size_t getActiveThreadCount() const; // threads currently running a task
    size_t getNumThreads()      const { return numThreads; }
    bool   isRunning()          const { return running.load(std::memory_order_acquire); }

    /// Change pool size (stop + restart with new count).
    void setNumThreads(size_t n);

    ThreadManager(const ThreadManager&)            = delete;
    ThreadManager& operator=(const ThreadManager&) = delete;

private:
    void workerThread(size_t id);        // main loop for each worker
    bool popTask(std::function<void()>&); // helper: get next job or return false

    // --- data ---
    size_t                       numThreads;
    std::vector<std::thread>     threads;

    std::queue<std::function<void()>> taskQueue;
    mutable std::mutex           queueMtx;
    std::condition_variable      queueCv;

    std::atomic<bool>            running {false};
    std::atomic<size_t>          activeThreads {0};
};
