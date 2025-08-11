#ifndef ECOSYSTEM_PROJECT_INCLUDE_UTILS_RANDOM_H_
#define ECOSYSTEM_PROJECT_INCLUDE_UTILS_RANDOM_H_

#include <random>

namespace Ecosystem {
namespace Utils {
namespace Random {

/// @brief Get thread-local random number generator
/// @return Reference to thread-local MT19937 generator
inline std::mt19937& GetGenerator() {
  thread_local std::random_device rd;
  thread_local std::mt19937 gen(rd());
  return gen;
}

/// @brief Generate random integer in range [min, max]
/// @param min Minimum value (inclusive)
/// @param max Maximum value (inclusive)
/// @return Random integer in range
inline int RandomInt(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(GetGenerator());
}

/// @brief Generate random float in range [min, max)
/// @param min Minimum value (inclusive)
/// @param max Maximum value (exclusive)
/// @return Random float in range
inline float RandomFloat(float min, float max) {
  std::uniform_real_distribution<float> dist(min, max);
  return dist(GetGenerator());
}

/// @brief Generate random boolean with given probability
/// @param probability Probability of returning true [0.0, 1.0]
/// @return Random boolean
inline bool RandomBool(float probability = 0.5f) {
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  return dist(GetGenerator()) < probability;
}

/// @brief Shuffle container elements
/// @tparam Container Type of container to shuffle
/// @param container Container to shuffle
template <typename Container>
inline void Shuffle(Container& container) {
  std::shuffle(container.begin(), container.end(), GetGenerator());
}

}  // namespace Random
}  // namespace Utils
}  // namespace Ecosystem

#endif  // ECOSYSTEM_PROJECT_INCLUDE_UTILS_RANDOM_H_