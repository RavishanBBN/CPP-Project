// include/ThreadManager.h
#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <queue>

/// A simple thread‐pool manager that executes tasks in parallel.
class ThreadManager {
public:
    /// Construct with a given number of worker threads.
    explicit ThreadManager(size_t numThreads);
    ~ThreadManager();

    /// Start all worker threads.
    void start();

    /// Stop processing: finish queued tasks, then join threads.
    void stop();

    /// Block until queue is empty and no thread is active.
    void waitForCompletion();

    /// Enqueue a new task.
    void addTask(std::function<void()> task);

    /// Number of tasks currently waiting.
    size_t getTaskCount() const;

    /// Get and set the number of worker threads.
    size_t getNumThreads() const;
    void setNumThreads(size_t numThreads);

    /// Number of threads actively executing tasks.
    size_t getActiveThreadCount() const;

    /// Is the pool currently running?
    bool isRunning() const;

private:
    /// Worker thread main loop.
    void workerThread();

    mutable std::mutex queueMutex_;
    std::condition_variable taskCv_;
    std::queue<std::function<void()>> taskQueue_;

    std::vector<std::thread> threads_;
    std::atomic<bool> running_{false};
    std::atomic<size_t> activeCount_{0};
    size_t threadCount_;
};
