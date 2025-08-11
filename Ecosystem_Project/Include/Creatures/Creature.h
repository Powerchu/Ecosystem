#ifndef _CREATURE_H_
#define _CREATURE_H_

#include <functional>
#include <queue>
#include <string>

#include "EcoSystem/Terrain.h"

#define MAX_CREATURE_SPEED 10.f
#define MAX_CREATURE_SIZE 10.f
#define MAX_CREATURE_SENSE 10.f

namespace Ecosystem {
extern float CREATURE_MUTATION_EPSILON;

struct EvolutionData {
  EvolutionData(float _repT = 0.9f, float _repl = 0.f,
                float _muta = 0.f) noexcept;
  EvolutionData(const EvolutionData& _evo) noexcept;
  EvolutionData& operator=(const EvolutionData& _evo) noexcept;
  float mfReplicationThresh;  // threshhold to attempt to replicate
  float mfReplicateChance;    // actual birthing low value
  float mfMutationChance;     // when replicating theres mutating chance
};

struct Traits {
  Traits(float _size, float _spd, float _sense) noexcept;
  Traits(const Traits& _t) noexcept;
  float mfSize;
  float mfSense;
  float mfSpeed;
};

class Creature {
 public:
  enum Flags : unsigned short {
    FLAG_INVALID = 0,

    FLAG_DEAD = 1 << 15
  };

  Creature(const std::string& _name, unsigned short _flags, const Traits& _t,
           unsigned _id) noexcept;
  virtual ~Creature(void) noexcept;

  // getters
  unsigned short GetFlags(void) const noexcept;
  float GetSize(void) const noexcept;
  float GetSpeed(void) const noexcept;
  float GetSense(void) const noexcept;
  unsigned int GetColor(void) const noexcept;
  const std::string& GetName(void) const noexcept;
  std::pair<float, float> GetFatigue(void) const noexcept;
  std::pair<float, float> GetEnergy(void) const noexcept;
  void GetGridPosition(unsigned& _outX, unsigned& _outY) const noexcept;
  Ecosystem::GridPos GetGridPosition(void) const noexcept;
  void GetHomeGridPosition(unsigned& _outX, unsigned& _outY) const noexcept;
  bool HasPendingMovement(void) const noexcept;
  Ecosystem::GridPos GetPendingDestination(void) const noexcept;
  const std::string& GetUniqueID(void) const noexcept;
  float GetMutChance(void) const noexcept;
  float GetRepChance(void) const noexcept;
  const EvolutionData& GetEvoData(void) const noexcept;
  const Traits& GetTraits(void) const noexcept;

  void SetFatigueBase(const std::pair<float, float>& _curMax) noexcept;
  void SetEnergyBase(const std::pair<float, float>& _curMax) noexcept;

  // threshhold before flagging the creature on rate 0.f - 1.f (flags for tired
  // under fatigue and starving under energy)
  void SetFatigueThreshold(float _zeroToOne) noexcept;
  void SetEnergyThreshold(float _zeroToOne) noexcept;

  // to be used at user end (performing actions like hunting?)
  float ConsumeFatigue(float) noexcept;
  float ConsumeEnergy(float) noexcept;

  // set color to be represented on grid
  void SetColor(float _red = 1.f, float _green = 1.f, float _blue = 1.f,
                float _alpha = 1.f) noexcept;

  // set position
  void SetGridPosition(unsigned _x, unsigned _y) noexcept;

  // mark curr position as new home base
  void MarkTerritory(void) noexcept;

  // set evolution data
  void SetEvolutionData(const EvolutionData& _dat) noexcept;

  // update creature primitive state - also calls user defined behaviour
  void UpdateAwake(float) noexcept;
  void UpdateAsleep(float) noexcept;

  // To move creature
  void SetMovement(const std::vector<GridPos>& _path);

  // fun functions
  float Eaten(Creature* _predator);
  float Eat(void);
  void Replicate(void);

  // TODO : USER-END TO IMPLEMENT
  virtual void UpdateAwakeBehaviour(float) = 0;
  virtual void UpdateAsleepBehaviour(float) = 0;

 protected:
  unsigned short mBitFlags;

 private:
  std::string mName;
  std::string mUniqueID;
  unsigned int mnColorCode;
  unsigned mnChartID;

  // first is current, second is max
  std::pair<float, float> mfFatigue;
  // first is current, second is max
  std::pair<float, float> mfEnergy;

  float mfFatigueThresh;
  float mfEnergyThresh;

  Traits mTraits;
  EvolutionData mEvoData;

  // current path to move
  std::deque<GridPos> mCurPath;
  float mfAccPathDt;

  unsigned mnPosX;
  unsigned mnPosY;
  unsigned mnHomeX;
  unsigned mnHomeY;

  void Move(float);
};

}  // namespace Ecosystem

#endif
