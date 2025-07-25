#ifndef THREAD_SAFE_H_
#define THREAD_SAFE_H_

#include <queue>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include "util.h" // for vertex_ID_t

// Thread-safe queue for BFS
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        condition_.notify_one();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty(); });
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    // Swap member function
    void swap(ThreadSafeQueue& other) {
        if (this == &other) return;
        std::scoped_lock lock(mutex_, other.mutex_);
        queue_.swap(other.queue_);
    }
};

// Non-member swap overload for ThreadSafeQueue
template<typename T>
void swap(ThreadSafeQueue<T>& a, ThreadSafeQueue<T>& b) {
    a.swap(b);
}

// Thread-safe vector for distances
class ThreadSafeDistances {
private:
    std::vector<std::atomic<long long>> distances_;
    std::vector<std::atomic<bool>> visited_;

public:
    ThreadSafeDistances(size_t size) : distances_(size), visited_(size) {
        for (size_t i = 0; i < size; ++i) {
            distances_[i].store(-1, std::memory_order_relaxed);
            visited_[i].store(false, std::memory_order_relaxed);
        }
    }

    bool try_visit(vertex_ID_t vertex, long long distance) {
        bool expected = false;
        if (visited_[vertex].compare_exchange_strong(expected, true, 
                                                   std::memory_order_acquire, 
                                                   std::memory_order_relaxed)) {
            distances_[vertex].store(distance, std::memory_order_relaxed);
            return true;
        }
        return false;
    }

    long long get_distance(vertex_ID_t vertex) const {
        return distances_[vertex].load(std::memory_order_relaxed);
    }

    bool is_visited(vertex_ID_t vertex) const {
        return visited_[vertex].load(std::memory_order_relaxed);
    }
};

#endif // THREAD_SAFE_H_ 