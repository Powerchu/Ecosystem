#ifndef ECOSYSTEM_PROJECT_INCLUDE_ECOSYSTEM_PARALLEL_INTEGRATION_H_
#define ECOSYSTEM_PROJECT_INCLUDE_ECOSYSTEM_PARALLEL_INTEGRATION_H_

#include "EcoSystem/ParallelEcoSystem.h"
#include "Creatures/ThreadSafeCreatures.h"

namespace Ecosystem {

/// @brief Integration layer for parallel ecosystem features
class ParallelIntegration {
 public:
  /// @brief Initialize parallel ecosystem with optimal settings
  /// @param enable_parallel Enable parallel processing
  /// @param thread_count Number of threads (0 = auto-detect)
  static void InitializeSystem(bool enable_parallel = true, size_t thread_count = 0);

  /// @brief Update ecosystem with automatic parallel/single-threaded selection
  /// @param dt Delta time
  static void UpdateEcosystem(float dt);

  /// @brief Check if parallel processing is active
  /// @return True if using parallel processing
  static bool IsParallelActive();

  /// @brief Get performance statistics
  /// @return Performance statistics
  static ParallelPerformanceMonitor::Stats GetPerformanceStats();

  /// @brief Enable/disable performance monitoring
  /// @param enabled Enable performance monitoring
  static void SetPerformanceMonitoring(bool enabled);

  /// @brief Create thread-safe creature with automatic type detection
  /// @param creature_type Type of creature ("Rabbit", "Fox", etc.)
  /// @param traits Creature traits
  /// @param position Initial position
  /// @return Pointer to created creature (nullptr if failed)
  static Creature* CreateThreadSafeCreature(const std::string& creature_type,
                                           const Traits& traits,
                                           const GridPos& position);

  /// @brief Batch creature creation for performance
  /// @param creature_requests Vector of creation requests
  /// @return Vector of created creatures
  struct CreatureRequest {
    std::string type;
    Traits traits;
    GridPos position;
  };
  static std::vector<Creature*> CreateCreaturesBatch(
      const std::vector<CreatureRequest>& requests);

  /// @brief Thread-safe environmental queries
  static float GetEnvironmentQuality(const GridPos& position, float radius);
  static std::vector<GridPos> FindOptimalSpawnLocations(size_t count, 
                                                       const std::string& creature_type);

 private:
  static bool parallel_enabled_;
  static bool performance_monitoring_;
  static std::unique_ptr<ParallelEcoSystem> parallel_ecosystem_;
};

/// @brief Performance benchmarking utilities
class ParallelBenchmark {
 public:
  struct BenchmarkResult {
    double single_threaded_time;
    double parallel_time;
    double speedup_factor;
    size_t creature_count;
    size_t thread_count;
    double parallel_efficiency;
  };

  /// @brief Run performance benchmark comparing single vs parallel
  /// @param creature_count Number of creatures to test with
  /// @param duration_seconds How long to run the benchmark
  /// @return Benchmark results
  static BenchmarkResult RunBenchmark(size_t creature_count, double duration_seconds = 5.0);

  /// @brief Test different thread counts for optimal performance
  /// @param max_threads Maximum thread count to test
  /// @return Vector of results for each thread count
  static std::vector<BenchmarkResult> FindOptimalThreadCount(size_t max_threads = 16);

  /// @brief Memory usage profiling
  struct MemoryProfile {
    size_t peak_memory_mb;
    size_t average_memory_mb;
    size_t allocation_count;
    double fragmentation_ratio;
  };
  
  /// @brief Profile memory usage during parallel execution
  /// @param duration_seconds Duration to profile
  /// @return Memory usage statistics
  static MemoryProfile ProfileMemoryUsage(double duration_seconds = 10.0);
};

}  // namespace Ecosystem

#endif  // ECOSYSTEM_PROJECT_INCLUDE_ECOSYSTEM_PARALLEL_INTEGRATION_H_