#ifndef ECOSYSTEM_PROJECT_INCLUDE_ECOSYSTEM_PARALLEL_ECOSYSTEM_H_
#define ECOSYSTEM_PROJECT_INCLUDE_ECOSYSTEM_PARALLEL_ECOSYSTEM_H_

#include "EcoSystem/EcoSystem.h"
#include "Utils/Threading.h"
#include <array>
#include <unordered_map>
#include <unordered_set>

namespace Ecosystem {

/// @brief Thread-safe parallel ecosystem with spatial partitioning
class ParallelEcoSystem : public EcoSystem {
 public:
  static ParallelEcoSystem& GetInst() noexcept;

  /// @brief Initialize parallel ecosystem and create tools
  void Init() noexcept;

  /// @brief Initialize parallel system with thread count
  /// @param num_threads Number of worker threads (0 = auto-detect)
  void InitializeParallel(size_t num_threads = 0);

  /// @brief Parallel update with proper synchronization
  /// @param dt Delta time
  void ParallelUpdate(float dt) noexcept;

  /// @brief Thread-safe creature sensing
  /// @param creature Creature doing the sensing (can be any Creature* - thread safety ensured by implementation)
  /// @param sense_range Sensing range
  /// @param creature_types Types to sense for (optional filter)
  /// @return Vector of nearby creatures (base Creature* for polymorphism)
  /// @note Returns base Creature* pointers for polymorphism - caller should apply thread-safe behaviors
  std::vector<Creature*> SenseNearbyCreatures(
      const Creature* creature, float sense_range,
      const std::unordered_set<std::string>& creature_types = {}) const;

  /// @brief Thread-safe grass access
  /// @param x Grid X coordinate
  /// @param y Grid Y coordinate
  /// @return Grass value (thread-safe)
  float GetGrassValueThreadSafe(unsigned x, unsigned y) const;

  /// @brief Thread-safe grass consumption
  /// @param x Grid X coordinate  
  /// @param y Grid Y coordinate
  /// @param amount Amount to consume
  /// @return Actual amount consumed
  float ConsumeGrassThreadSafe(unsigned x, unsigned y, float amount);

  /// @brief Thread-safe predation attempt
  /// @param predator Predator creature
  /// @param prey Prey creature
  /// @return Energy gained from successful predation (0 if failed)
  float AttemptPredationThreadSafe(Creature* predator, Creature* prey);

 private:
  ParallelEcoSystem() : EcoSystem(64, 64, 32), thread_pool_(nullptr), initialized_(false) {}

  // Threading infrastructure
  std::unique_ptr<Utils::Threading::ThreadPool> thread_pool_;
  std::unique_ptr<Utils::Threading::SpatialPartition> spatial_partition_;
  bool initialized_;

  // Synchronization for shared resources
  mutable Utils::Threading::ReadWriteLock terrain_lock_;
  mutable Utils::Threading::ReadWriteLock creatures_lock_;
  mutable std::vector<std::unique_ptr<std::mutex>> grid_mutexes_;  // Fine-grained locking

  // Spatial partitioning data
  static constexpr unsigned PARTITION_SIZE = 16;  // 16x16 grid partitions
  std::vector<std::vector<Creature*>> partition_creatures_;
  std::vector<std::unique_ptr<std::mutex>> partition_mutexes_;

  // Thread-safe data structures
  Utils::Threading::AtomicValue<size_t> active_creatures_;
  Utils::Threading::AtomicValue<size_t> total_energy_;

  // Parallel processing methods
  void ParallelUpdateCreatures(float dt);
  void ParallelUpdateTerrain(float dt);
  void ParallelCleanupDead();
  void UpdateSpatialPartitions();

  // Synchronization helpers
  void LockPartitionsForCreature(const Creature* creature,
                                 std::vector<std::unique_lock<std::mutex>>& locks) const;
  size_t GetGridMutexIndex(unsigned x, unsigned y) const;

  // Conflict resolution
  struct InteractionRequest {
    enum Type { PREDATION, GRASS_CONSUMPTION, MOVEMENT };
    Type type;
    Creature* creature1;
    Creature* creature2;  // nullptr for non-creature interactions
    unsigned x, y;
    float amount;
    std::promise<float> result;
  };

  std::queue<InteractionRequest> interaction_queue_;
  std::mutex interaction_mutex_;
  
  void ProcessInteractions();
};

/// @brief Thread-safe creature update interface
class ThreadSafeCreature {
 public:
  /// @brief Update creature in parallel context
  /// @param creature Creature to update
  /// @param dt Delta time
  /// @param ecosystem Reference to parallel ecosystem
  static void ParallelUpdate(Creature* creature, float dt, 
                           ParallelEcoSystem& ecosystem);

  /// @brief Check if creature can move to position safely
  /// @param creature Creature attempting move
  /// @param x Target X coordinate
  /// @param y Target Y coordinate
  /// @param ecosystem Reference to parallel ecosystem
  /// @return True if move is safe
  static bool CanMoveSafely(const Creature* creature, unsigned x, unsigned y,
                          const ParallelEcoSystem& ecosystem);

 private:
  ThreadSafeCreature() = delete;
};

/// @brief Performance monitoring for parallel ecosystem
class ParallelPerformanceMonitor {
 public:
  struct Stats {
    double total_update_time;
    double creature_update_time;
    double terrain_update_time;
    double synchronization_overhead;
    size_t active_threads;
    size_t creatures_per_thread;
    double parallel_efficiency;  // 0.0 to 1.0
  };

  static Stats GetStats();
  static void ResetStats();
  static void RecordUpdateTime(const std::string& phase, double time_ms);

 private:
  static Stats current_stats_;
  static std::mutex stats_mutex_;
  static std::chrono::high_resolution_clock::time_point last_update_;
};

}  // namespace Ecosystem

#endif  // ECOSYSTEM_PROJECT_INCLUDE_ECOSYSTEM_PARALLEL_ECOSYSTEM_H_