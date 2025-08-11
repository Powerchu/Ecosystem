#include "Utils/UUID.h"

#include <random>
#include <sstream>

namespace Ecosystem {
namespace Utils {

std::string UUID::Generate() {
  // Create a random device and generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> hex_dist(0, 15);
  std::uniform_int_distribution<> variant_dist(8, 11);

  // Generate UUID4 format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
  // where x is any hexadecimal digit and y is one of 8, 9, A, or B
  std::ostringstream oss;
  oss << std::hex;

  // First group: 8 hex digits
  for (int i = 0; i < 8; ++i) {
    oss << hex_dist(gen);
  }
  oss << "-";

  // Second group: 4 hex digits
  for (int i = 0; i < 4; ++i) {
    oss << hex_dist(gen);
  }
  oss << "-";

  // Third group: 4xxx (version 4)
  oss << "4";
  for (int i = 0; i < 3; ++i) {
    oss << hex_dist(gen);
  }
  oss << "-";

  // Fourth group: yxxx (variant bits)
  oss << variant_dist(gen);
  for (int i = 0; i < 3; ++i) {
    oss << hex_dist(gen);
  }
  oss << "-";

  // Fifth group: 12 hex digits
  for (int i = 0; i < 12; ++i) {
    oss << hex_dist(gen);
  }

  return oss.str();
}

}  // namespace Utils
}  // namespace Ecosystem