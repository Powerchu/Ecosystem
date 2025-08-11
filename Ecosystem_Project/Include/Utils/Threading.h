#ifndef ECOSYSTEM_PROJECT_INCLUDE_UTILS_THREADING_H_
#define ECOSYSTEM_PROJECT_INCLUDE_UTILS_THREADING_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Ecosystem {
namespace Utils {
namespace Threading {

/// @brief Thread-safe work queue for task distribution
class ThreadPool {
 public:
  explicit ThreadPool(size_t num_threads);
  ~ThreadPool();

  /// @brief Submit a task to the thread pool
  /// @tparam F Function type
  /// @tparam Args Argument types
  /// @param f Function to execute
  /// @param args Arguments to pass
  /// @return Future for the result
  template <typename F, typename... Args>
  auto Submit(F&& f, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> result = task->get_future();

    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      if (stop_) {
        throw std::runtime_error("Submit called on stopped ThreadPool");
      }
      tasks_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return result;
  }

  /// @brief Wait for all current tasks to complete
  void WaitForAll();

  /// @brief Get number of worker threads
  size_t GetThreadCount() const { return workers_.size(); }

 private:
  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::atomic<bool> stop_;
  std::atomic<size_t> active_tasks_;
  std::condition_variable all_done_;
};

/// @brief Spatial partitioning for parallel creature updates
class SpatialPartition {
 public:
  SpatialPartition(unsigned width, unsigned height, unsigned partition_size);

  /// @brief Get partition ID for given coordinates
  /// @param x X coordinate
  /// @param y Y coordinate
  /// @return Partition ID
  size_t GetPartitionId(unsigned x, unsigned y) const;

  /// @brief Get number of partitions
  size_t GetPartitionCount() const { return partition_count_; }

  /// @brief Get partition bounds
  /// @param partition_id Partition ID
  /// @return {min_x, min_y, max_x, max_y}
  std::array<unsigned, 4> GetPartitionBounds(size_t partition_id) const;

  /// @brief Check if two partitions are neighbors (for boundary interactions)
  bool AreNeighbors(size_t partition1, size_t partition2) const;

 private:
  unsigned width_, height_;
  unsigned partition_size_;
  unsigned partitions_x_, partitions_y_;
  size_t partition_count_;
};

/// @brief Read-Write lock for shared resources
class ReadWriteLock {
 public:
  ReadWriteLock() : readers_(0), writers_(0), write_requests_(0) {}

  /// @brief Acquire read lock (multiple readers allowed)
  void ReadLock();

  /// @brief Release read lock
  void ReadUnlock();

  /// @brief Acquire write lock (exclusive access)
  void WriteLock();

  /// @brief Release write lock
  void WriteUnlock();

  /// @brief RAII read lock guard
  class ReadGuard {
   public:
    explicit ReadGuard(ReadWriteLock& lock) : lock_(lock) { lock_.ReadLock(); }
    ~ReadGuard() { lock_.ReadUnlock(); }

   private:
    ReadWriteLock& lock_;
  };

  /// @brief RAII write lock guard
  class WriteGuard {
   public:
    explicit WriteGuard(ReadWriteLock& lock) : lock_(lock) {
      lock_.WriteLock();
    }
    ~WriteGuard() { lock_.WriteUnlock(); }

   private:
    ReadWriteLock& lock_;
  };

 private:
  std::mutex mutex_;
  std::condition_variable read_condition_;
  std::condition_variable write_condition_;
  std::atomic<int> readers_;
  std::atomic<int> writers_;
  std::atomic<int> write_requests_;
};

/// @brief Lock-free atomic operations for performance-critical sections
template <typename T>
class AtomicValue {
 public:
  AtomicValue(T initial_value = T{}) : value_(initial_value) {}

  T Load() const { return value_.load(std::memory_order_acquire); }

  void Store(T new_value) { value_.store(new_value, std::memory_order_release); }

  bool CompareExchange(T& expected, T desired) {
    return value_.compare_exchange_weak(expected, desired,
                                        std::memory_order_acq_rel);
  }

  T FetchAdd(T increment) {
    return value_.fetch_add(increment, std::memory_order_acq_rel);
  }

 private:
  std::atomic<T> value_;
};

}  // namespace Threading
}  // namespace Utils
}  // namespace Ecosystem

#endif  // ECOSYSTEM_PROJECT_INCLUDE_UTILS_THREADING_H_