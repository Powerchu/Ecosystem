#ifndef ECOSYSTEM_PROJECT_INCLUDE_UTILS_UUID_H_
#define ECOSYSTEM_PROJECT_INCLUDE_UTILS_UUID_H_

#include <string>

namespace Ecosystem {
namespace Utils {

/// @brief Utility class for UUID generation
class UUID {
 public:
  /// @brief Generate a random UUID4 string
  /// @return A UUID4 formatted string (xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx)
  static std::string Generate();

 private:
  UUID() = delete;  // Static utility class
};

}  // namespace Utils
}  // namespace Ecosystem

#endif  // ECOSYSTEM_PROJECT_INCLUDE_UTILS_UUID_H_