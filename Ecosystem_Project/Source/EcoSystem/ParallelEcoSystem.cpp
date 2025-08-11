#include "EcoSystem/ParallelEcoSystem.h"
#include "Creatures/Creature.h"
#include "Creatures/ThreadSafeCreatures.h"
#include "Utils/Math.h"
#include "Utils/Random.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>
#include <iostream>

namespace Ecosystem {

// Static initialization
ParallelPerformanceMonitor::Stats ParallelPerformanceMonitor::current_stats_{};
std::mutex ParallelPerformanceMonitor::stats_mutex_;
std::chrono::high_resolution_clock::time_point 
    ParallelPerformanceMonitor::last_update_ = std::chrono::high_resolution_clock::now();

ParallelEcoSystem& ParallelEcoSystem::GetInst() noexcept {
  static ParallelEcoSystem instance;
  return instance;
}

void ParallelEcoSystem::InitializeParallel(size_t num_threads) {
  if (initialized_) return;

  // Auto-detect thread count if not specified
  if (num_threads == 0) {
    num_threads = std::max(2u, std::thread::hardware_concurrency());
  }

  // Initialize thread pool
  thread_pool_ = std::make_unique<Utils::Threading::ThreadPool>(num_threads);

  // Initialize spatial partitioning
  spatial_partition_ = std::make_unique<Utils::Threading::SpatialPartition>(
      GetWidth(), GetHeight(), PARTITION_SIZE);

  // Initialize partition data structures
  const size_t partition_count = spatial_partition_->GetPartitionCount();
  partition_creatures_.resize(partition_count);
  partition_mutexes_.resize(partition_count);
  for (size_t i = 0; i < partition_count; ++i) {
    partition_mutexes_[i] = std::make_unique<std::mutex>();
  }

  // Initialize fine-grained grid mutexes (one per 4x4 grid section)
  const size_t grid_mutex_count = (GetWidth() * GetHeight()) / 16;
  grid_mutexes_.resize(grid_mutex_count);
  for (size_t i = 0; i < grid_mutex_count; ++i) {
    grid_mutexes_[i] = std::make_unique<std::mutex>();
  }

  initialized_ = true;

  // Initialize performance monitoring
  ParallelPerformanceMonitor::ResetStats();
}

void ParallelEcoSystem::ParallelUpdate(float dt) noexcept {
  if (!initialized_) {
    InitializeParallel();
  }

  auto start_time = std::chrono::high_resolution_clock::now();

  if (mbRunEco) {
    // Phase 1: Update spatial partitions
    UpdateSpatialPartitions();

    // Phase 2: Parallel terrain update
    ParallelUpdateTerrain(dt);

    // Phase 3: Parallel creature updates
    ParallelUpdateCreatures(dt);

    // Phase 4: Process interactions (serialized for consistency)
    ProcessInteractions();

    // Phase 5: Cleanup dead creatures
    ParallelCleanupDead();

    // Phase 6: Update tools and logs (existing single-threaded code)
    if (mbEcoTool) EcoTool();
    UpdateTools();

    mfLogAccDt += mfDelta;
    if (mfLogAccDt > 1.f / mfLogFreq) UpdateLogs();

    RenderMap();
  } else {
    RenderSetup();
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time).count() / 1000.0;
  
  ParallelPerformanceMonitor::RecordUpdateTime("total", duration);
}

void ParallelEcoSystem::UpdateSpatialPartitions() {
  // Clear all partition creature lists
  for (auto& partition : partition_creatures_) {
    partition.clear();
  }

  // Redistribute creatures to partitions
  Utils::Threading::ReadWriteLock::ReadGuard creatures_guard(creatures_lock_);
  
  for (auto* creature : mAllCreatures) {
    if (creature->GetFlags() & Creature::FLAG_DEAD) continue;

    unsigned x, y;
    creature->GetGridPosition(x, y);
    const size_t partition_id = spatial_partition_->GetPartitionId(x, y);
    
    std::lock_guard<std::mutex> partition_guard(*partition_mutexes_[partition_id]);
    partition_creatures_[partition_id].push_back(creature);
  }

  // Update atomic counters
  active_creatures_.Store(mAllCreatures.size());
}

void ParallelEcoSystem::ParallelUpdateTerrain(float dt) {
  auto start_time = std::chrono::high_resolution_clock::now();

  // Divide terrain into chunks for parallel processing
  const size_t partition_count = spatial_partition_->GetPartitionCount();
  std::vector<std::future<void>> terrain_futures;

  for (size_t partition_id = 0; partition_id < partition_count; ++partition_id) {
    terrain_futures.push_back(thread_pool_->Submit([this, partition_id, dt]() {
      auto bounds = spatial_partition_->GetPartitionBounds(partition_id);
      const unsigned min_x = bounds[0], min_y = bounds[1];
      const unsigned max_x = bounds[2], max_y = bounds[3];

      // Update grass growth in this partition
      Utils::Threading::ReadWriteLock::WriteGuard terrain_guard(terrain_lock_);
      
      auto& grass_layer = mTerrain.GetGrassLayer();
      auto& fertilizer_layer = mTerrain.GetFertilizerLayer();
      const auto& grass_thresh = mTerrain.GetGrassLayerThresh();
      const auto& fertilizer_thresh = mTerrain.GetFertilizerLayerThresh();

      for (unsigned y = min_y; y < max_y; ++y) {
        for (unsigned x = min_x; x < max_x; ++x) {
          // Grass growth calculation using inherited member variables
          const float growth_rate = Utils::Math::FastLerp(
              mfGRateLo, mfGRateHi, 
              fertilizer_layer[y][x] / fertilizer_thresh[y][x].second);

          grass_layer[y][x] = std::min(
              grass_thresh[y][x].second,
              grass_layer[y][x] + growth_rate * dt);

          // Fertilizer decay
          const float decay_rate = Utils::Math::FastLerp(
              mfFRateLo, mfFRateHi,
              fertilizer_layer[y][x] / fertilizer_thresh[y][x].second);

          fertilizer_layer[y][x] = std::max(
              0.0f, fertilizer_layer[y][x] - decay_rate * dt);
        }
      }
    }));
  }

  // Wait for all terrain updates to complete
  for (auto& future : terrain_futures) {
    future.wait();
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time).count() / 1000.0;
  ParallelPerformanceMonitor::RecordUpdateTime("terrain", duration);
}

void ParallelEcoSystem::ParallelUpdateCreatures(float dt) {
  auto start_time = std::chrono::high_resolution_clock::now();

  const size_t partition_count = spatial_partition_->GetPartitionCount();
  std::vector<std::future<void>> creature_futures;

  // Process each partition in parallel
  for (size_t partition_id = 0; partition_id < partition_count; ++partition_id) {
    creature_futures.push_back(thread_pool_->Submit([this, partition_id, dt]() {
      std::lock_guard<std::mutex> partition_guard(*partition_mutexes_[partition_id]);
      
      for (auto* creature : partition_creatures_[partition_id]) {
        if (creature->GetFlags() & Creature::FLAG_DEAD) continue;
        
        // Apply type-specific thread-safe behaviors
        if (auto* rabbit = ThreadSafeRabbit::AsRabbit(creature)) {
          ThreadSafeRabbit::ParallelUpdateBehavior(rabbit, dt, *this);
        } else if (auto* fox = ThreadSafeFox::AsFox(creature)) {
          ThreadSafeFox::ParallelUpdateBehavior(fox, dt, *this);
        } else {
          // Fallback to generic thread-safe update
          ThreadSafeCreature::ParallelUpdate(creature, dt, *this);
        }
      }
    }));
  }

  // Wait for all creature updates to complete
  for (auto& future : creature_futures) {
    future.wait();
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time).count() / 1000.0;
  ParallelPerformanceMonitor::RecordUpdateTime("creatures", duration);
}

std::vector<Creature*> ParallelEcoSystem::SenseNearbyCreatures(
    const Creature* creature, float sense_range,
    const std::unordered_set<std::string>& creature_types) const {
  
  std::vector<Creature*> nearby_creatures;
  
  unsigned creature_x, creature_y;
  creature->GetGridPosition(creature_x, creature_y);
  
  const float sense_range_sq = sense_range * sense_range;
  const size_t creature_partition = spatial_partition_->GetPartitionId(creature_x, creature_y);

  // Check current partition and neighboring partitions
  Utils::Threading::ReadWriteLock::ReadGuard creatures_guard(creatures_lock_);
  
  for (size_t partition_id = 0; partition_id < spatial_partition_->GetPartitionCount(); ++partition_id) {
    // Skip distant partitions for efficiency
    if (!spatial_partition_->AreNeighbors(creature_partition, partition_id) && 
        partition_id != creature_partition) {
      continue;
    }

    std::lock_guard<std::mutex> partition_guard(*partition_mutexes_[partition_id]);
    
    for (auto* other_creature : partition_creatures_[partition_id]) {
      if (other_creature == creature || 
          (other_creature->GetFlags() & Creature::FLAG_DEAD)) {
        continue;
      }

      // Filter by creature type if specified
      if (!creature_types.empty() && 
          creature_types.find(other_creature->GetName()) == creature_types.end()) {
        continue;
      }

      unsigned other_x, other_y;
      other_creature->GetGridPosition(other_x, other_y);
      
      const float dist_sq = Utils::Math::DistanceSquared(
          static_cast<float>(creature_x) - static_cast<float>(other_x),
          static_cast<float>(creature_y) - static_cast<float>(other_y));

      if (dist_sq <= sense_range_sq) {
        nearby_creatures.push_back(other_creature);
      }
    }
  }

  return nearby_creatures;
}

float ParallelEcoSystem::GetGrassValueThreadSafe(unsigned x, unsigned y) const {
  if (x >= static_cast<unsigned>(GetWidth()) || y >= static_cast<unsigned>(GetHeight())) return 0.0f;
  
  Utils::Threading::ReadWriteLock::ReadGuard terrain_guard(terrain_lock_);
  return mTerrain.GetGrassLayer()[y][x];
}

float ParallelEcoSystem::ConsumeGrassThreadSafe(unsigned x, unsigned y, float amount) {
  if (x >= static_cast<unsigned>(GetWidth()) || y >= static_cast<unsigned>(GetHeight()) || amount <= 0.0f) return 0.0f;

  const size_t mutex_index = GetGridMutexIndex(x, y);
  std::lock_guard<std::mutex> grid_guard(*grid_mutexes_[mutex_index]);
  
  auto& grass_layer = mTerrain.GetGrassLayer();
  const float available = grass_layer[y][x];
  const float consumed = std::min(amount, available);
  
  grass_layer[y][x] -= consumed;
  return consumed;
}

float ParallelEcoSystem::AttemptPredationThreadSafe(Creature* predator, Creature* prey) {
  if (!predator || !prey || 
      (predator->GetFlags() & Creature::FLAG_DEAD) ||
      (prey->GetFlags() & Creature::FLAG_DEAD)) {
    return 0.0f;
  }

  // Lock both creatures' partitions in consistent order to prevent deadlock
  unsigned pred_x, pred_y, prey_x, prey_y;
  predator->GetGridPosition(pred_x, pred_y);
  prey->GetGridPosition(prey_x, prey_y);

  const size_t pred_partition = spatial_partition_->GetPartitionId(pred_x, pred_y);
  const size_t prey_partition = spatial_partition_->GetPartitionId(prey_x, prey_y);

  std::vector<std::unique_lock<std::mutex>> locks;
  
  if (pred_partition == prey_partition) {
    locks.emplace_back(*partition_mutexes_[pred_partition]);
  } else {
    // Lock in consistent order to prevent deadlock
    const size_t first = std::min(pred_partition, prey_partition);
    const size_t second = std::max(pred_partition, prey_partition);
          locks.emplace_back(*partition_mutexes_[first]);
    locks.emplace_back(*partition_mutexes_[second]);
  }

  // Verify creatures are still alive and in proximity
  if ((predator->GetFlags() & Creature::FLAG_DEAD) ||
      (prey->GetFlags() & Creature::FLAG_DEAD)) {
    return 0.0f;
  }

  // Check distance (predators can only eat adjacent prey)
  const float dist_sq = Utils::Math::DistanceSquared(
      static_cast<float>(pred_x) - static_cast<float>(prey_x),
      static_cast<float>(pred_y) - static_cast<float>(prey_y));

  if (dist_sq > 2.0f) {  // Maximum adjacent distance (including diagonal)
    return 0.0f;
  }

  // Perform predation
  const float energy_gained = prey->Eaten(predator);
  predator->ConsumeEnergy(-energy_gained);  // Gain energy (negative consumption)

  return energy_gained;
}

void ParallelEcoSystem::ProcessInteractions() {
  // This method handles serialized conflict resolution
  // In practice, most interactions would be queued during parallel phase
  // and resolved here to maintain consistency
  
  std::lock_guard<std::mutex> interaction_guard(interaction_mutex_);
  
  while (!interaction_queue_.empty()) {
    auto& interaction = interaction_queue_.front();
    
    float result = 0.0f;
    
    switch (interaction.type) {
      case InteractionRequest::PREDATION:
        result = AttemptPredationThreadSafe(interaction.creature1, interaction.creature2);
        break;
        
      case InteractionRequest::GRASS_CONSUMPTION:
        result = ConsumeGrassThreadSafe(interaction.x, interaction.y, interaction.amount);
        break;
        
      case InteractionRequest::MOVEMENT:
        // Handle movement conflicts here
        break;
    }
    
    interaction.result.set_value(result);
    interaction_queue_.pop();
  }
}

void ParallelEcoSystem::ParallelCleanupDead() {
  Utils::Threading::ReadWriteLock::WriteGuard creatures_guard(creatures_lock_);
  
  // Use parallel scan to identify dead creatures
  std::vector<size_t> dead_indices;
  const size_t creature_count = mAllCreatures.size();
  
  // Parallel identification of dead creatures
  std::mutex dead_mutex;
  const size_t chunk_size = std::max(1ul, creature_count / thread_pool_->GetThreadCount());
  std::vector<std::future<void>> scan_futures;
  
  for (size_t start = 0; start < creature_count; start += chunk_size) {
    const size_t end = std::min(start + chunk_size, creature_count);
    
    scan_futures.push_back(thread_pool_->Submit([this, start, end, &dead_indices, &dead_mutex]() {
      std::vector<size_t> local_dead;
      
      for (size_t i = start; i < end; ++i) {
        if (mAllCreatures[i]->GetFlags() & Creature::FLAG_DEAD) {
          local_dead.push_back(i);
        }
      }
      
      if (!local_dead.empty()) {
        std::lock_guard<std::mutex> guard(dead_mutex);
        dead_indices.insert(dead_indices.end(), local_dead.begin(), local_dead.end());
      }
    }));
  }
  
  // Wait for scanning to complete
  for (auto& future : scan_futures) {
    future.wait();
  }
  
  // Sort dead indices in reverse order for safe removal
  std::sort(dead_indices.rbegin(), dead_indices.rend());
  
  // Remove dead creatures (single-threaded for consistency)
  auto& space_layer = mTerrain.GetSpaceLayer();
  auto& fertilizer_layer = mTerrain.GetFertilizerLayer();
  
  for (size_t dead_index : dead_indices) {
    auto* creature = mAllCreatures[dead_index];
    const auto pos = creature->GetGridPosition();
    
    // Check if creature died naturally (not eaten)
    if (space_layer[pos.y][pos.x] == static_cast<int>(dead_index)) {
      space_layer[pos.y][pos.x] = -1;
    }
    
    // Add remaining energy as fertilizer
    fertilizer_layer[pos.y][pos.x] += 
        creature->GetEnergy().second * mfDeathThresh;
    
    // Remove creature efficiently
    if (dead_index != mAllCreatures.size() - 1) {
      std::swap(mAllCreatures[dead_index], mAllCreatures.back());
    }
    
    delete creature;
    mAllCreatures.pop_back();
  }
}

size_t ParallelEcoSystem::GetGridMutexIndex(unsigned x, unsigned y) const {
  // Map grid coordinates to mutex index (group 4x4 cells per mutex)
  const unsigned mutex_x = x / 4;
  const unsigned mutex_y = y / 4;
  const unsigned mutexes_per_row = (static_cast<unsigned>(GetWidth()) + 3) / 4;
  return mutex_y * mutexes_per_row + mutex_x;
}

// ThreadSafeCreature implementations moved to ThreadSafeCreatures.cpp

// Performance Monitor Implementation
ParallelPerformanceMonitor::Stats ParallelPerformanceMonitor::GetStats() {
  std::lock_guard<std::mutex> guard(stats_mutex_);
  return current_stats_;
}

void ParallelPerformanceMonitor::ResetStats() {
  std::lock_guard<std::mutex> guard(stats_mutex_);
  current_stats_ = Stats{};
  last_update_ = std::chrono::high_resolution_clock::now();
}

void ParallelPerformanceMonitor::RecordUpdateTime(const std::string& phase, double time_ms) {
  std::lock_guard<std::mutex> guard(stats_mutex_);
  
  if (phase == "total") {
    current_stats_.total_update_time = time_ms;
  } else if (phase == "creatures") {
    current_stats_.creature_update_time = time_ms;
  } else if (phase == "terrain") {
    current_stats_.terrain_update_time = time_ms;
  }
  
  // Calculate efficiency metrics
  const double parallel_time = current_stats_.creature_update_time + 
                              current_stats_.terrain_update_time;
  const double sequential_time = current_stats_.total_update_time;
  
  if (sequential_time > 0) {
    current_stats_.parallel_efficiency = parallel_time / sequential_time;
  }
}

}  // namespace Ecosystem