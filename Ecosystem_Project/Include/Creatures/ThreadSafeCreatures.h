#ifndef ECOSYSTEM_PROJECT_INCLUDE_CREATURES_THREAD_SAFE_CREATURES_H_
#define ECOSYSTEM_PROJECT_INCLUDE_CREATURES_THREAD_SAFE_CREATURES_H_

#include "Creatures/Fox.h"
#include "Creatures/Rabbit.h"
#include "EcoSystem/ParallelEcoSystem.h"
#include <memory>
#include <unordered_set>

namespace Ecosystem {

/* ARCHITECTURE NOTE: Thread-Safe Creature Design
 * 
 * This system uses a Strategy Pattern approach for thread safety:
 * 
 * 1. Base Creature* Polymorphism:
 *    - SenseNearbyCreatures() returns base Creature* for polymorphism
 *    - ThreadSafeCreature methods accept base Creature* parameters
 *    - This allows sensing any creature type through common interface
 * 
 * 2. Thread-Safe Behavior Strategies:
 *    - ThreadSafeRabbit/Fox classes provide static strategy methods
 *    - These apply thread-safe behaviors to existing Creature objects
 *    - No need to change base Creature class or inheritance hierarchy
 * 
 * 3. Type Safety:
 *    - AsRabbit()/AsFox() provide safe downcasting with type checking
 *    - Methods verify creature type before applying behaviors
 *    - Null checks ensure robustness
 * 
 * 4. Benefits:
 *    - Preserves existing Creature hierarchy
 *    - Thread-safe operations without changing base classes
 *    - Polymorphic sensing with type-safe behavior application
 *    - Easy to extend with new creature types
 */

/// @brief Thread-safe rabbit behavior implementation
/// @note Uses base Creature* for polymorphism but ensures thread-safe operations
class ThreadSafeRabbit {
 public:
  /// @brief Thread-safe update for rabbit behavior
  /// @param rabbit Rabbit instance to update (base Creature* for polymorphism)
  /// @param dt Delta time
  /// @param ecosystem Parallel ecosystem reference
  static void ParallelUpdateBehavior(Rabbit* rabbit, float dt, 
                                   ParallelEcoSystem& ecosystem);

  /// @brief Safe type conversion for thread-safe operations
  /// @param creature Base creature pointer
  /// @return Rabbit pointer if valid, nullptr otherwise
  static Rabbit* AsRabbit(Creature* creature) {
    return (creature && creature->GetName() == "Rabbit") ? 
           static_cast<Rabbit*>(creature) : nullptr;
  }

 private:
  /// @brief Thread-safe grass seeking behavior
  static void SeekGrassParallel(Rabbit* rabbit, ParallelEcoSystem& ecosystem);
  
  /// @brief Thread-safe predator avoidance
  static void AvoidPredatorsParallel(Rabbit* rabbit, ParallelEcoSystem& ecosystem);
  
  /// @brief Thread-safe random movement when no objectives
  static void RandomMovementParallel(Rabbit* rabbit, ParallelEcoSystem& ecosystem);
};

/// @brief Thread-safe fox behavior implementation
/// @note Uses base Creature* for polymorphism but ensures thread-safe operations  
class ThreadSafeFox {
 public:
  /// @brief Thread-safe update for fox behavior
  /// @param fox Fox instance to update (base Creature* for polymorphism)
  /// @param dt Delta time
  /// @param ecosystem Parallel ecosystem reference
  static void ParallelUpdateBehavior(Fox* fox, float dt, 
                                   ParallelEcoSystem& ecosystem);

  /// @brief Safe type conversion for thread-safe operations
  /// @param creature Base creature pointer
  /// @return Fox pointer if valid, nullptr otherwise
  static Fox* AsFox(Creature* creature) {
    return (creature && creature->GetName() == "Fox") ? 
           static_cast<Fox*>(creature) : nullptr;
  }

 private:
  /// @brief Thread-safe prey hunting behavior
  static void HuntPreyParallel(Fox* fox, ParallelEcoSystem& ecosystem);
  
  /// @brief Thread-safe territorial behavior
  static void DefendTerritoryParallel(Fox* fox, ParallelEcoSystem& ecosystem);
  
  /// @brief Thread-safe exploration when no prey found
  static void ExploreParallel(Fox* fox, ParallelEcoSystem& ecosystem);
};

/// @brief Parallel pathfinding with thread safety
class ParallelPathfinder {
 public:
  /// @brief Thread-safe pathfinding request
  /// @param ecosystem Ecosystem reference
  /// @param start Start position
  /// @param goal Goal position
  /// @param max_search_distance Maximum search distance
  /// @return Path vector (empty if no path found)
  static std::vector<GridPos> FindPathThreadSafe(
      const ParallelEcoSystem& ecosystem,
      const GridPos& start, const GridPos& goal,
      float max_search_distance = 50.0f);

  /// @brief Thread-safe best resource position finding
  /// @param ecosystem Ecosystem reference
  /// @param start Start position
  /// @param resource_type Type of resource to find
  /// @param search_radius Search radius
  /// @param min_quality Minimum resource quality
  /// @return Best resource position
  static GridPos FindBestResourceThreadSafe(
      const ParallelEcoSystem& ecosystem,
      const GridPos& start, const std::string& resource_type,
      float search_radius, float min_quality = 0.3f);

 private:
  /// @brief Lock-free heuristic calculation for A*
  static float CalculateHeuristic(const GridPos& from, const GridPos& to);
  
  /// @brief Thread-safe neighbor expansion
  static std::vector<GridPos> GetValidNeighborsThreadSafe(
      const ParallelEcoSystem& ecosystem, const GridPos& pos);
};

/// @brief Lock-free interaction cache for performance
class InteractionCache {
 public:
  struct CachedInteraction {
    std::chrono::steady_clock::time_point timestamp;
    std::vector<Creature*> nearby_creatures;
    float grass_density;
    bool has_predator_threat;
  };

  /// @brief Get cached interaction data
  /// @param creature_id Unique creature identifier
  /// @return Cached interaction data (nullptr if not cached or expired)
  static const CachedInteraction* GetCachedData(const std::string& creature_id);

  /// @brief Update cached interaction data
  /// @param creature_id Unique creature identifier
  /// @param data New interaction data
  static void UpdateCache(const std::string& creature_id, const CachedInteraction& data);

  /// @brief Clear expired cache entries
  static void CleanupExpiredEntries();

 private:
  static std::unordered_map<std::string, CachedInteraction> cache_;
  static std::mutex cache_mutex_;
  static constexpr std::chrono::milliseconds CACHE_DURATION{100};  // 100ms cache
};

/// @brief Parallel reproduction system
class ParallelReproduction {
 public:
  struct ReproductionRequest {
    Creature* parent;
    GridPos location;
    Traits offspring_traits;
    std::promise<bool> success_promise;
  };

  /// @brief Queue reproduction request for processing
  /// @param request Reproduction request
  /// @return Future indicating success/failure
  static std::future<bool> QueueReproduction(ReproductionRequest&& request);

  /// @brief Process all queued reproduction requests
  /// @param ecosystem Ecosystem reference
  static void ProcessReproductionQueue(ParallelEcoSystem& ecosystem);

 private:
  static std::queue<ReproductionRequest> reproduction_queue_;
  static std::mutex reproduction_mutex_;
  
  /// @brief Find safe reproduction location near parent
  static GridPos FindSafeReproductionSite(const Creature* parent, 
                                         const ParallelEcoSystem& ecosystem);
};

/// @brief Memory pool for creatures to reduce allocation overhead
template<typename CreatureType>
class CreaturePool {
 public:
  /// @brief Get creature from pool or create new one
  /// @param args Constructor arguments
  /// @return Unique pointer to creature
  template<typename... Args>
  static std::unique_ptr<CreatureType> AcquireCreature(Args&&... args);

  /// @brief Return creature to pool for reuse
  /// @param creature Creature to return (will be reset)
  static void ReleaseCreature(std::unique_ptr<CreatureType> creature);

  /// @brief Get current pool statistics
  /// @return Pair of (available, total) creatures in pool
  static std::pair<size_t, size_t> GetPoolStats();

 private:
  static std::vector<std::unique_ptr<CreatureType>> available_creatures_;
  static std::mutex pool_mutex_;
  static constexpr size_t MAX_POOL_SIZE = 1000;
};

}  // namespace Ecosystem

#endif  // ECOSYSTEM_PROJECT_INCLUDE_CREATURES_THREAD_SAFE_CREATURES_H_