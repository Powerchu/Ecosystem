#ifndef _RABBIT_H_
#define _RABBIT_H_
#include "Creature.h"

namespace Ecosystem {
class Rabbit : public Creature {
 public:
  Rabbit(const Traits& _t, unsigned _id) noexcept;
  ~Rabbit(void) noexcept;

  void UpdateAwakeBehaviour(float);
  void UpdateAsleepBehaviour(float);

 private:
  bool searching;
  [[maybe_unused]] bool predFound;
};
}  // namespace Ecosystem

#endif
