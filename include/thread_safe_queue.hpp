#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "frame.hpp"

class ThreadSafeQueue {
private:
    std::queue<std::unique_ptr<Frame>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
    bool is_shutdown_ = false;

public:
    ThreadSafeQueue() = default;

    // Push a new frame into the queue (uses move semantics)
    void push(std::unique_ptr<Frame> frame) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (is_shutdown_) return;
            queue_.push(std::move(frame));
        }
        cond_var_.notify_one(); // Wake up the waiting consumer
    }

    // Wait for and pop a frame. Returns false if queue is shut down.
    bool wait_and_pop(std::unique_ptr<Frame>& out_frame) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until the queue is not empty OR we are shutting down
        cond_var_.wait(lock, [this] { return !queue_.empty() || is_shutdown_; });

        if (is_shutdown_ && queue_.empty()) {
            return false;
        }

        out_frame = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // Signal all threads to stop waiting and exit cleanly
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            is_shutdown_ = true;
        }
        cond_var_.notify_all(); // Wake up all waiting threads so they can exit
    }
};