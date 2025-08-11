#include "Creatures/ThreadSafeCreatures.h"
#include "Utils/Math.h"
#include "Utils/Random.h"
#include <algorithm>
#include <chrono>

namespace Ecosystem {

// Static member initialization
std::unordered_map<std::string, InteractionCache::CachedInteraction> InteractionCache::cache_;
std::mutex InteractionCache::cache_mutex_;

std::queue<ParallelReproduction::ReproductionRequest> ParallelReproduction::reproduction_queue_;
std::mutex ParallelReproduction::reproduction_mutex_;

// ThreadSafeRabbit Implementation
void ThreadSafeRabbit::ParallelUpdateBehavior(Rabbit* rabbit, float /*dt*/, 
                                             ParallelEcoSystem& ecosystem) {
  if (!rabbit || (rabbit->GetFlags() & Creature::FLAG_DEAD)) return;

  // Check cache first for performance
  const auto* cached_data = InteractionCache::GetCachedData(rabbit->GetUniqueID());
  
  bool has_predator_threat = false;
  float local_grass_density = 0.0f;
  
  if (cached_data && 
      std::chrono::steady_clock::now() - cached_data->timestamp < 
      std::chrono::milliseconds(50)) {
    // Use cached data
    has_predator_threat = cached_data->has_predator_threat;
    local_grass_density = cached_data->grass_density;
  } else {
    // Perform sensing and update cache
    unsigned rabbit_x, rabbit_y;
    rabbit->GetGridPosition(rabbit_x, rabbit_y);
    
    // Sense nearby foxes (predators)
    std::unordered_set<std::string> predator_types{"Fox"};
    auto nearby_predators = ecosystem.SenseNearbyCreatures(
        rabbit, rabbit->GetSense(), predator_types);
    has_predator_threat = !nearby_predators.empty();
    
    // Sample local grass density
    const float sample_radius = 3.0f;
    float total_grass = 0.0f;
    int sample_count = 0;
    
    for (int dy = -static_cast<int>(sample_radius); dy <= static_cast<int>(sample_radius); ++dy) {
      for (int dx = -static_cast<int>(sample_radius); dx <= static_cast<int>(sample_radius); ++dx) {
        const unsigned sample_x = rabbit_x + dx;
        const unsigned sample_y = rabbit_y + dy;
        
        if (sample_x < static_cast<unsigned>(ecosystem.GetWidth()) && sample_y < static_cast<unsigned>(ecosystem.GetHeight())) {
          total_grass += ecosystem.GetGrassValueThreadSafe(sample_x, sample_y);
          sample_count++;
        }
      }
    }
    
    local_grass_density = (sample_count > 0) ? (total_grass / sample_count) : 0.0f;
    
    // Update cache
    InteractionCache::CachedInteraction cache_data;
    cache_data.timestamp = std::chrono::steady_clock::now();
    cache_data.grass_density = local_grass_density;
    cache_data.has_predator_threat = has_predator_threat;
    InteractionCache::UpdateCache(rabbit->GetUniqueID(), cache_data);
  }
  
  // Behavior decision making
  if (has_predator_threat) {
    AvoidPredatorsParallel(rabbit, ecosystem);
  } else if (local_grass_density > 0.1f) {
    // Eat local grass if available
    unsigned x, y;
    rabbit->GetGridPosition(x, y);
    const float grass_consumed = ecosystem.ConsumeGrassThreadSafe(x, y, 0.5f);
    if (grass_consumed > 0) {
      rabbit->ConsumeEnergy(-grass_consumed * 10.0f);  // Gain energy
    }
  } else if (rabbit->GetEnergy().first < rabbit->GetEnergy().second * 0.6f) {
    SeekGrassParallel(rabbit, ecosystem);
  } else {
    RandomMovementParallel(rabbit, ecosystem);
  }
}

void ThreadSafeRabbit::SeekGrassParallel(Rabbit* rabbit, ParallelEcoSystem& ecosystem) {
  unsigned rabbit_x, rabbit_y;
  rabbit->GetGridPosition(rabbit_x, rabbit_y);
  
  const GridPos start{static_cast<int>(rabbit_x), static_cast<int>(rabbit_y)};
  const GridPos grass_location = ParallelPathfinder::FindBestResourceThreadSafe(
      ecosystem, start, "grass", rabbit->GetSense(), 0.3f);
  
  if (grass_location.x != -1 && grass_location.y != -1) {
    auto path = ParallelPathfinder::FindPathThreadSafe(ecosystem, start, grass_location);
    if (!path.empty()) {
      rabbit->SetMovement(path);
    }
  }
}

void ThreadSafeRabbit::AvoidPredatorsParallel(Rabbit* rabbit, ParallelEcoSystem& ecosystem) {
  unsigned rabbit_x, rabbit_y;
  rabbit->GetGridPosition(rabbit_x, rabbit_y);
  
  // Find nearby predators
  std::unordered_set<std::string> predator_types{"Fox"};
  auto nearby_predators = ecosystem.SenseNearbyCreatures(
      rabbit, rabbit->GetSense(), predator_types);
  
  if (nearby_predators.empty()) return;
  
  // Calculate escape vector (average direction away from all predators)
  float escape_x = 0.0f, escape_y = 0.0f;
  
  for (const auto* predator : nearby_predators) {
    unsigned pred_x, pred_y;
    predator->GetGridPosition(pred_x, pred_y);
    
    const float dx = static_cast<float>(rabbit_x) - static_cast<float>(pred_x);
    const float dy = static_cast<float>(rabbit_y) - static_cast<float>(pred_y);
    const float dist = Utils::Math::FastSqrt(dx * dx + dy * dy) + 0.001f;  // Avoid division by zero
    
    escape_x += dx / dist;
    escape_y += dy / dist;
  }
  
  // Normalize escape vector
  const float escape_mag = Utils::Math::FastSqrt(escape_x * escape_x + escape_y * escape_y) + 0.001f;
  escape_x /= escape_mag;
  escape_y /= escape_mag;
  
  // Find escape destination
  const float escape_distance = 10.0f;
  const int target_x = Utils::Math::FastClamp(
      static_cast<int>(rabbit_x + escape_x * escape_distance),
      0, static_cast<int>(ecosystem.GetWidth() - 1));
  const int target_y = Utils::Math::FastClamp(
      static_cast<int>(rabbit_y + escape_y * escape_distance),
      0, static_cast<int>(ecosystem.GetHeight() - 1));
  
  const GridPos start{static_cast<int>(rabbit_x), static_cast<int>(rabbit_y)};
  const GridPos escape_target{target_x, target_y};
  
  auto escape_path = ParallelPathfinder::FindPathThreadSafe(ecosystem, start, escape_target);
  if (!escape_path.empty()) {
    rabbit->SetMovement(escape_path);
  }
}

void ThreadSafeRabbit::RandomMovementParallel(Rabbit* rabbit, ParallelEcoSystem& ecosystem) {
  unsigned rabbit_x, rabbit_y;
  rabbit->GetGridPosition(rabbit_x, rabbit_y);
  
  // Random movement in exploration mode
  const int range = 5;
  const int target_x = Utils::Math::FastClamp(
      static_cast<int>(rabbit_x) + Utils::Random::RandomInt(-range, range),
      0, static_cast<int>(ecosystem.GetWidth() - 1));
  const int target_y = Utils::Math::FastClamp(
      static_cast<int>(rabbit_y) + Utils::Random::RandomInt(-range, range),
      0, static_cast<int>(ecosystem.GetHeight() - 1));
  
  const GridPos start{static_cast<int>(rabbit_x), static_cast<int>(rabbit_y)};
  const GridPos target{target_x, target_y};
  
  auto path = ParallelPathfinder::FindPathThreadSafe(ecosystem, start, target);
  if (!path.empty()) {
    rabbit->SetMovement(path);
  }
}

// ThreadSafeFox Implementation
void ThreadSafeFox::ParallelUpdateBehavior(Fox* fox, float /*dt*/, 
                                         ParallelEcoSystem& ecosystem) {
  if (!fox || (fox->GetFlags() & Creature::FLAG_DEAD)) return;

  // Check energy level to determine behavior
  const float energy_ratio = fox->GetEnergy().first / fox->GetEnergy().second;
  
  if (energy_ratio < 0.3f) {
    // Hungry - actively hunt
    HuntPreyParallel(fox, ecosystem);
  } else if (energy_ratio > 0.8f) {
    // Well-fed - defend territory or reproduce
    DefendTerritoryParallel(fox, ecosystem);
  } else {
    // Moderate energy - explore
    ExploreParallel(fox, ecosystem);
  }
}

void ThreadSafeFox::HuntPreyParallel(Fox* fox, ParallelEcoSystem& ecosystem) {
  unsigned fox_x, fox_y;
  fox->GetGridPosition(fox_x, fox_y);
  
  // Sense nearby prey
  std::unordered_set<std::string> prey_types{"Rabbit"};
  auto nearby_prey = ecosystem.SenseNearbyCreatures(
      fox, fox->GetSense(), prey_types);
  
  if (nearby_prey.empty()) {
    // No nearby prey, search for prey trails or move to high-density areas
    const GridPos start{static_cast<int>(fox_x), static_cast<int>(fox_y)};
    const GridPos prey_area = ParallelPathfinder::FindBestResourceThreadSafe(
        ecosystem, start, "prey_trail", fox->GetSense() * 2.0f, 0.1f);
    
    if (prey_area.x != -1) {
      auto path = ParallelPathfinder::FindPathThreadSafe(ecosystem, start, prey_area);
      if (!path.empty()) {
        fox->SetMovement(path);
      }
    }
    return;
  }
  
  // Find closest prey
  Creature* closest_prey = nullptr;
  float closest_distance_sq = std::numeric_limits<float>::max();
  
  for (auto* prey : nearby_prey) {
    unsigned prey_x, prey_y;
    prey->GetGridPosition(prey_x, prey_y);
    
    const float dist_sq = Utils::Math::DistanceSquared(
        static_cast<float>(fox_x) - static_cast<float>(prey_x),
        static_cast<float>(fox_y) - static_cast<float>(prey_y));
    
    if (dist_sq < closest_distance_sq) {
      closest_distance_sq = dist_sq;
      closest_prey = prey;
    }
  }
  
  if (closest_prey) {
    // If prey is adjacent, attempt predation
    if (closest_distance_sq <= 2.0f) {  // Adjacent (including diagonal)
      const float energy_gained = ecosystem.AttemptPredationThreadSafe(fox, closest_prey);
      if (energy_gained > 0) {
        // Successful hunt - stay in place briefly
        return;
      }
    }
    
    // Move toward prey
    unsigned prey_x, prey_y;
    closest_prey->GetGridPosition(prey_x, prey_y);
    
    const GridPos start{static_cast<int>(fox_x), static_cast<int>(fox_y)};
    const GridPos prey_pos{static_cast<int>(prey_x), static_cast<int>(prey_y)};
    
    auto hunt_path = ParallelPathfinder::FindPathThreadSafe(ecosystem, start, prey_pos);
    if (!hunt_path.empty()) {
      fox->SetMovement(hunt_path);
    }
  }
}

void ThreadSafeFox::DefendTerritoryParallel(Fox* fox, ParallelEcoSystem& ecosystem) {
  // Check for other foxes in territory
  std::unordered_set<std::string> competitor_types{"Fox"};
  auto nearby_foxes = ecosystem.SenseNearbyCreatures(
      fox, fox->GetSense(), competitor_types);
  
  // Remove self from list
  nearby_foxes.erase(
      std::remove(nearby_foxes.begin(), nearby_foxes.end(), fox),
      nearby_foxes.end());
  
  if (!nearby_foxes.empty()) {
    // Chase away competitors (move toward them aggressively)
    auto* competitor = nearby_foxes[0];
    unsigned comp_x, comp_y, fox_x, fox_y;
    competitor->GetGridPosition(comp_x, comp_y);
    fox->GetGridPosition(fox_x, fox_y);
    
    const GridPos start{static_cast<int>(fox_x), static_cast<int>(fox_y)};
    const GridPos competitor_pos{static_cast<int>(comp_x), static_cast<int>(comp_y)};
    
    auto chase_path = ParallelPathfinder::FindPathThreadSafe(ecosystem, start, competitor_pos);
    if (!chase_path.empty()) {
      fox->SetMovement(chase_path);
    }
  } else {
    // Territory is secure - consider reproduction
    // (Implementation would queue reproduction request)
  }
}

void ThreadSafeFox::ExploreParallel(Fox* fox, ParallelEcoSystem& ecosystem) {
  unsigned fox_x, fox_y;
  fox->GetGridPosition(fox_x, fox_y);
  
  // Patrol territory in search of opportunities
  const int patrol_range = 8;
  const int target_x = Utils::Math::FastClamp(
      static_cast<int>(fox_x) + Utils::Random::RandomInt(-patrol_range, patrol_range),
      0, static_cast<int>(ecosystem.GetWidth() - 1));
  const int target_y = Utils::Math::FastClamp(
      static_cast<int>(fox_y) + Utils::Random::RandomInt(-patrol_range, patrol_range),
      0, static_cast<int>(ecosystem.GetHeight() - 1));
  
  const GridPos start{static_cast<int>(fox_x), static_cast<int>(fox_y)};
  const GridPos target{target_x, target_y};
  
  auto patrol_path = ParallelPathfinder::FindPathThreadSafe(ecosystem, start, target);
  if (!patrol_path.empty()) {
    fox->SetMovement(patrol_path);
  }
}

// ParallelPathfinder Implementation
std::vector<GridPos> ParallelPathfinder::FindPathThreadSafe(
    const ParallelEcoSystem& ecosystem,
    const GridPos& start, const GridPos& goal,
    float max_search_distance) {
  
  // Simple thread-safe A* implementation
  // For full implementation, would use lock-free data structures
  // and careful synchronization
  
  std::vector<GridPos> path;
  
  // Quick distance check
  const float dist_to_goal = Utils::Math::OctileDistance(
      static_cast<float>(goal.x - start.x),
      static_cast<float>(goal.y - start.y));
  
  if (dist_to_goal > max_search_distance) {
    return path;  // Too far, return empty path
  }
  
  // For now, return a simple direct path (simplified)
  // Full implementation would use proper A* with thread-safe terrain access
  const int dx = (goal.x > start.x) ? 1 : (goal.x < start.x) ? -1 : 0;
  const int dy = (goal.y > start.y) ? 1 : (goal.y < start.y) ? -1 : 0;
  
  GridPos current = start;
  int steps = 0;
  const int max_steps = static_cast<int>(max_search_distance);
  
  while (current.x != goal.x || current.y != goal.y) {
    if (steps++ > max_steps) break;
    
    current.x += dx;
    current.y += dy;
    
    // Bounds check
    if (current.x < 0 || current.x >= ecosystem.GetWidth() ||
        current.y < 0 || current.y >= ecosystem.GetHeight()) {
      break;
    }
    
    path.push_back(current);
  }
  
  return path;
}

GridPos ParallelPathfinder::FindBestResourceThreadSafe(
    const ParallelEcoSystem& ecosystem,
    const GridPos& start, const std::string& resource_type,
    float search_radius, float min_quality) {
  
  GridPos best_location{-1, -1};
  float best_quality = min_quality;
  
  const int radius = static_cast<int>(search_radius);
  
  for (int dy = -radius; dy <= radius; ++dy) {
    for (int dx = -radius; dx <= radius; ++dx) {
      const int check_x = start.x + dx;
      const int check_y = start.y + dy;
      
      if (check_x < 0 || check_x >= ecosystem.GetWidth() ||
          check_y < 0 || check_y >= ecosystem.GetHeight()) {
        continue;
      }
      
      float quality = 0.0f;
      if (resource_type == "grass") {
        quality = ecosystem.GetGrassValueThreadSafe(check_x, check_y);
      }
      // Add other resource types as needed
      
      if (quality > best_quality) {
        best_quality = quality;
        best_location = GridPos{check_x, check_y};
      }
    }
  }
  
  return best_location;
}

// InteractionCache Implementation
const InteractionCache::CachedInteraction* InteractionCache::GetCachedData(
    const std::string& creature_id) {
  std::lock_guard<std::mutex> guard(cache_mutex_);
  
  auto it = cache_.find(creature_id);
  if (it != cache_.end()) {
    const auto& cached = it->second;
    const auto now = std::chrono::steady_clock::now();
    
    if (now - cached.timestamp < CACHE_DURATION) {
      return &cached;
    } else {
      cache_.erase(it);  // Remove expired entry
    }
  }
  
  return nullptr;
}

void InteractionCache::UpdateCache(const std::string& creature_id, 
                                 const CachedInteraction& data) {
  std::lock_guard<std::mutex> guard(cache_mutex_);
  cache_[creature_id] = data;
}

void InteractionCache::CleanupExpiredEntries() {
  std::lock_guard<std::mutex> guard(cache_mutex_);
  
  const auto now = std::chrono::steady_clock::now();
  
  auto it = cache_.begin();
  while (it != cache_.end()) {
    if (now - it->second.timestamp >= CACHE_DURATION) {
      it = cache_.erase(it);
    } else {
      ++it;
    }
  }
}

// ThreadSafeCreature Implementation
void ThreadSafeCreature::ParallelUpdate(Creature* creature, float dt, 
                                       ParallelEcoSystem& /*ecosystem*/) {
  if (!creature || dt <= 0.0f) return;
  
  // Basic energy consumption (thread-safe as it only modifies creature's own state)
  creature->UpdateAwake(dt);
  
  // Thread-safe sensing and decision making would go here
  // For now, delegate to existing single-threaded behavior methods
}

bool ThreadSafeCreature::CanMoveSafely(const Creature* creature, unsigned x, unsigned y,
                                      const ParallelEcoSystem& ecosystem) {
  if (!creature) return false;
  
  // Check bounds
  if (x >= static_cast<unsigned>(ecosystem.GetWidth()) || y >= static_cast<unsigned>(ecosystem.GetHeight())) {
    return false;
  }
  
  // For now, use simple thread-safe check through ecosystem API
  // Full implementation would use fine-grained grid locking
  return ecosystem.GetGridVal(x, y) == -1;
}

}  // namespace Ecosystem