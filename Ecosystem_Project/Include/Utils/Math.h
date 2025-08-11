#ifndef ECOSYSTEM_PROJECT_INCLUDE_UTILS_MATH_H_
#define ECOSYSTEM_PROJECT_INCLUDE_UTILS_MATH_H_

#include <cmath>

namespace Ecosystem {
namespace Utils {
namespace Math {

/// @brief Fast approximate square root using bit manipulation
/// @param x Input value
/// @return Approximate square root (good for game/simulation use)
inline float FastSqrt(float x) {
  // Carmack's fast inverse square root adapted for square root
  union {
    float f;
    uint32_t i;
  } conv = {x};
  conv.i = 0x5f3759df - (conv.i >> 1);
  conv.f *= 0.5f * (3.0f - x * conv.f * conv.f);
  return x * conv.f;
}

/// @brief Fast distance calculation without sqrt (for comparisons)
/// @param dx X difference
/// @param dy Y difference
/// @return Squared distance (faster than sqrt for comparisons)
inline float DistanceSquared(float dx, float dy) { return dx * dx + dy * dy; }

/// @brief Fast Manhattan distance (L1 norm)
/// @param dx X difference
/// @param dy Y difference
/// @return Manhattan distance (sum of absolute differences)
inline float ManhattanDistance(float dx, float dy) {
  return std::abs(dx) + std::abs(dy);
}

/// @brief Fast octile distance for pathfinding (A* heuristic)
/// @param dx X difference
/// @param dy Y difference
/// @return Octile distance optimized for grid pathfinding
inline float OctileDistance(float dx, float dy) {
  const float abs_dx = std::abs(dx);
  const float abs_dy = std::abs(dy);
  return (abs_dx > abs_dy) ? 0.41421356f * abs_dy + abs_dx
                           : 0.41421356f * abs_dx + abs_dy;
}

/// @brief Fast interpolation without division
/// @param a Start value
/// @param b End value
/// @param t Interpolation factor [0,1]
/// @return Interpolated value
template <typename T>
inline T FastLerp(T a, T b, float t) {
  return a + (b - a) * t;
}

/// @brief Clamp value to range without branching
/// @param value Input value
/// @param min_val Minimum value
/// @param max_val Maximum value
/// @return Clamped value
template <typename T>
inline T FastClamp(T value, T min_val, T max_val) {
  return (value < min_val) ? min_val : (value > max_val) ? max_val : value;
}

/// @brief Fast power of 2 check
/// @param x Input value
/// @return True if x is a power of 2
inline bool IsPowerOf2(uint32_t x) { return x && !(x & (x - 1)); }

/// @brief Fast modulo for power of 2 divisors
/// @param value Input value
/// @param mod Modulo (must be power of 2)
/// @return value % mod (optimized for power of 2)
inline uint32_t FastMod(uint32_t value, uint32_t mod) {
  return value & (mod - 1);
}

}  // namespace Math
}  // namespace Utils
}  // namespace Ecosystem

#endif  // ECOSYSTEM_PROJECT_INCLUDE_UTILS_MATH_H_