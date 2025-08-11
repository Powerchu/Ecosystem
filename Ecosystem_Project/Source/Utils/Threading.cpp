#include "Utils/Threading.h"
#include <algorithm>

namespace Ecosystem {
namespace Utils {
namespace Threading {

// ThreadPool Implementation
ThreadPool::ThreadPool(size_t num_threads) : stop_(false), active_tasks_(0) {
  for (size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back([this] {
      for (;;) {
        std::function<void()> task;

        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

          if (stop_ && tasks_.empty()) return;

          task = std::move(tasks_.front());
          tasks_.pop();
          active_tasks_++;
        }

        task();

        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          active_tasks_--;
          if (active_tasks_ == 0 && tasks_.empty()) {
            all_done_.notify_all();
          }
        }
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    stop_ = true;
  }
  condition_.notify_all();
  for (std::thread& worker : workers_) {
    worker.join();
  }
}

// Template implementation moved to header file for proper instantiation

void ThreadPool::WaitForAll() {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  all_done_.wait(lock, [this] { return tasks_.empty() && active_tasks_ == 0; });
}

// SpatialPartition Implementation
SpatialPartition::SpatialPartition(unsigned width, unsigned height,
                                   unsigned partition_size)
    : width_(width),
      height_(height),
      partition_size_(partition_size),
      partitions_x_((width + partition_size - 1) / partition_size),
      partitions_y_((height + partition_size - 1) / partition_size),
      partition_count_(partitions_x_ * partitions_y_) {}

size_t SpatialPartition::GetPartitionId(unsigned x, unsigned y) const {
  const unsigned partition_x = x / partition_size_;
  const unsigned partition_y = y / partition_size_;
  return partition_y * partitions_x_ + partition_x;
}

std::array<unsigned, 4> SpatialPartition::GetPartitionBounds(
    size_t partition_id) const {
  const unsigned partition_x = partition_id % partitions_x_;
  const unsigned partition_y = partition_id / partitions_x_;

  const unsigned min_x = partition_x * partition_size_;
  const unsigned min_y = partition_y * partition_size_;
  const unsigned max_x = std::min(min_x + partition_size_, width_);
  const unsigned max_y = std::min(min_y + partition_size_, height_);

  return {min_x, min_y, max_x, max_y};
}

bool SpatialPartition::AreNeighbors(size_t partition1,
                                    size_t partition2) const {
  const unsigned x1 = partition1 % partitions_x_;
  const unsigned y1 = partition1 / partitions_x_;
  const unsigned x2 = partition2 % partitions_x_;
  const unsigned y2 = partition2 / partitions_x_;

  // Check if partitions are adjacent (including diagonally)
  return std::abs(static_cast<int>(x1) - static_cast<int>(x2)) <= 1 &&
         std::abs(static_cast<int>(y1) - static_cast<int>(y2)) <= 1;
}

// ReadWriteLock Implementation
void ReadWriteLock::ReadLock() {
  std::unique_lock<std::mutex> lock(mutex_);
  read_condition_.wait(lock, [this] { return writers_ == 0 && write_requests_ == 0; });
  readers_++;
}

void ReadWriteLock::ReadUnlock() {
  std::unique_lock<std::mutex> lock(mutex_);
  readers_--;
  if (readers_ == 0) {
    write_condition_.notify_all();
  }
}

void ReadWriteLock::WriteLock() {
  std::unique_lock<std::mutex> lock(mutex_);
  write_requests_++;
  write_condition_.wait(lock, [this] { return writers_ == 0 && readers_ == 0; });
  write_requests_--;
  writers_++;
}

void ReadWriteLock::WriteUnlock() {
  std::unique_lock<std::mutex> lock(mutex_);
  writers_--;
  read_condition_.notify_all();
  write_condition_.notify_all();
}

}  // namespace Threading
}  // namespace Utils
}  // namespace Ecosystem