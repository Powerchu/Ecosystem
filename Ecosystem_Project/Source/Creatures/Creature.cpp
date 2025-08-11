#include "Creatures/Creature.h"
#include "Data/EcoData.h"
#include "EcoSystem/EcoSystem.h"
#include "imgui.h"

#include <time.h>
#include <functional>
#include <random>
#include <utility>

namespace {
enum Action { MOVE, EAT, IDLE, REPLICATE, LAST };

static float gActionCost[Action::LAST] = {0.2f, 0.f, 0.0125f, 0.5f};

float GetActionCost(float size, float speed, float sense, float energy,
                    Action e, float _modifier = 1.f) {
  return (gActionCost[e] / 2 *
              ((2 * pow(size, 2) * speed * speed + sense + size) + energy) +
          (gActionCost[e] / 2) * energy) *
         _modifier;
}

std::string RandomString(int len) {
  std::srand(static_cast<unsigned int>(time(NULL)));
  std::string str =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  std::string newstr;
  int pos;
  while (newstr.size() != static_cast<size_t>(len)) {
    pos = ((std::rand() % (str.size() - 1)));
    newstr += str.substr(pos, 1);
  }
  return newstr;
}
}  // namespace

namespace Ecosystem {
extern EvolutionData EvolutionChart[EVOLUTION_CHART_COUNT];
}

float Ecosystem::CREATURE_MUTATION_EPSILON = 0.1f;

template <typename T>
T Clamp(T min, T max, T val) {
  return val > max ? max : val < min ? min : val;
}

Ecosystem::EvolutionData::EvolutionData(float _repT, float _repl,
                                        float _muta) noexcept
    : mfReplicationThresh{_repT},
      mfReplicateChance{_repl},
      mfMutationChance{_muta} {}

Ecosystem::EvolutionData::EvolutionData(const EvolutionData& _evo) noexcept
    : EvolutionData{_evo.mfReplicationThresh, _evo.mfReplicateChance,
                    _evo.mfMutationChance} {}

Ecosystem::EvolutionData& Ecosystem::EvolutionData::operator=(
    const EvolutionData& _evo) noexcept {
  if (this != &_evo) {
    mfReplicationThresh = _evo.mfReplicationThresh;
    mfReplicateChance = _evo.mfReplicateChance;
    mfMutationChance = _evo.mfMutationChance;
  }
  return *this;
}

Ecosystem::Traits::Traits(float _size, float _spd, float _sense) noexcept
    : mfSize{_size}, mfSense{_sense}, mfSpeed{_spd} {}

Ecosystem::Traits::Traits(const Traits& _t) noexcept
    : Traits{_t.mfSize, _t.mfSpeed, _t.mfSense} {}

Ecosystem::Creature::Creature(const std::string& _name, unsigned short _flags,
                              const Traits& _t, unsigned _id) noexcept
    : mBitFlags{_flags},
      mName{_name},
      mUniqueID{RandomString(16)},
      mnColorCode{},
      mnChartID{_id},
      mfFatigue{std::make_pair(0.f, 0.f)},
      mfEnergy{std::make_pair(0.f, 0.f)},
      mfFatigueThresh{0.3f},
      mfEnergyThresh{0.3f},
      mTraits{_t},
      mEvoData{},
      mCurPath{},
      mfAccPathDt{},
      mnPosX{},
      mnPosY{},
      mnHomeX{},
      mnHomeY{} {
  SetColor();
}

Ecosystem::Creature::~Creature(void) noexcept {}

unsigned short Ecosystem::Creature::GetFlags(void) const noexcept {
  return mBitFlags;
}

float Ecosystem::Creature::GetSize(void) const noexcept {
  return mTraits.mfSize;
}

float Ecosystem::Creature::GetSpeed(void) const noexcept {
  return mTraits.mfSpeed;
}

float Ecosystem::Creature::GetSense(void) const noexcept {
  return mTraits.mfSense;
}

unsigned int Ecosystem::Creature::GetColor(void) const noexcept {
  return mnColorCode;
}

const std::string& Ecosystem::Creature::GetName(void) const noexcept {
  return mName;
}

std::pair<float, float> Ecosystem::Creature::GetFatigue(void) const noexcept {
  return mfFatigue;
}

std::pair<float, float> Ecosystem::Creature::GetEnergy(void) const noexcept {
  return mfEnergy;
}

void Ecosystem::Creature::GetGridPosition(unsigned& _outX,
                                          unsigned& _outY) const noexcept {
  _outX = mnPosX;
  _outY = mnPosY;
}

Ecosystem::GridPos Ecosystem::Creature::GetGridPosition(void) const noexcept {
  return GridPos{static_cast<int>(mnPosX), static_cast<int>(mnPosY)};
}

void Ecosystem::Creature::GetHomeGridPosition(unsigned& _outX,
                                              unsigned& _outY) const noexcept {
  _outX = mnHomeX;
  _outY = mnHomeY;
}

bool Ecosystem::Creature::HasPendingMovement(void) const noexcept {
  return !mCurPath.empty();
}

Ecosystem::GridPos Ecosystem::Creature::GetPendingDestination(
    void) const noexcept {
  if (mCurPath.empty()) return GridPos{-1, -1};
  return mCurPath.back();
}

const std::string& Ecosystem::Creature::GetUniqueID(void) const noexcept {
  return mUniqueID;
}

float Ecosystem::Creature::GetMutChance(void) const noexcept {
  return mEvoData.mfMutationChance;
}

float Ecosystem::Creature::GetRepChance(void) const noexcept {
  return mEvoData.mfReplicateChance;
}

const Ecosystem::EvolutionData& Ecosystem::Creature::GetEvoData(
    void) const noexcept {
  return mEvoData;
}

const Ecosystem::Traits& Ecosystem::Creature::GetTraits(void) const noexcept {
  return mTraits;
}

void Ecosystem::Creature::SetFatigueThreshold(float _zeroToOne) noexcept {
  mfFatigueThresh = _zeroToOne;
}

void Ecosystem::Creature::SetEnergyThreshold(float _zeroToOne) noexcept {
  mfEnergyThresh = _zeroToOne;
}

void Ecosystem::Creature::SetFatigueBase(
    const std::pair<float, float>& _minMax) noexcept {
  mfFatigue = _minMax;
}

void Ecosystem::Creature::SetEnergyBase(
    const std::pair<float, float>& _minMax) noexcept {
  mfEnergy = _minMax;
}

float Ecosystem::Creature::ConsumeFatigue(float _f) noexcept {
  mfFatigue.first = Clamp(0.f, mfFatigue.second, mfFatigue.first - _f);
  return mfFatigue.first;
}

float Ecosystem::Creature::ConsumeEnergy(float _f) noexcept {
  if (_f > mfEnergy.first)
    EcoSystem::GetInst().ReturnEnergyToMap(_f - mfEnergy.first,
                                           GetGridPosition());
  mfEnergy.first = Clamp(0.f, mfEnergy.second, mfEnergy.first - _f);
  return mfEnergy.first;
}

void Ecosystem::Creature::SetColor(float _red, float _green, float _blue,
                                   float _alpha) noexcept {
  mnColorCode = ImGui::GetColorU32({_red, _green, _blue, _alpha});
}

void Ecosystem::Creature::SetGridPosition(unsigned _x, unsigned _y) noexcept {
  mnPosX = _x;
  mnPosY = _y;
}

void Ecosystem::Creature::MarkTerritory(void) noexcept {
  mnHomeX = mnPosX;
  mnHomeY = mnPosY;
}

void Ecosystem::Creature::SetEvolutionData(const EvolutionData& _dat) noexcept {
  mEvoData = _dat;
}

void Ecosystem::Creature::SetMovement(const std::vector<GridPos>& _path) {
  mCurPath.clear();
  for (const auto& p : _path) mCurPath.push_back(p);
  mfAccPathDt = 0.f;
}

void Ecosystem::Creature::Move(float _dt) {
  mfAccPathDt += _dt;

  int euc = (mCurPath.front().x - static_cast<int>(mnPosX)) *
                (mCurPath.front().x - static_cast<int>(mnPosX)) +
            (mCurPath.front().y - static_cast<int>(mnPosY)) *
                (mCurPath.front().y - static_cast<int>(mnPosY));

  float tReq = sqrt(static_cast<float>(euc)) / (mTraits.mfSpeed);

  while (mfAccPathDt > tReq) {
    mfAccPathDt -= tReq;
    SetGridPosition(mCurPath.front().x, mCurPath.front().y);
    ConsumeEnergy(::GetActionCost(mTraits.mfSize, mTraits.mfSpeed,
                                  mTraits.mfSense, mfEnergy.first,
                                  ::Action::MOVE));
    mCurPath.pop_front();
    if (mCurPath.empty()) break;

    euc = (mCurPath.front().x - static_cast<int>(mnPosX)) *
              (mCurPath.front().x - static_cast<int>(mnPosX)) +
          (mCurPath.front().y - static_cast<int>(mnPosY)) *
              (mCurPath.front().y - static_cast<int>(mnPosY));
    tReq = sqrt(static_cast<float>(euc)) / (mTraits.mfSpeed);
  }
}

float Ecosystem::Creature::Eaten(Creature*) {
  mBitFlags |= FLAG_DEAD;
  float e = mfEnergy.first;
  mfEnergy.first = 0;
  mfEnergy.second = 0;
  return e;
}

float Ecosystem::Creature::Eat(void) {
  float v = EcoSystem::GetInst().Eat(GetGridPosition(), this);
  if (mfEnergy.first + v > mfEnergy.second)
    EcoSystem::GetInst().ReturnEnergyToMap(mfEnergy.first + v - mfEnergy.second,
                                           GetGridPosition());
  mfEnergy.first = Clamp(mfEnergy.first, mfEnergy.second, mfEnergy.first + v);
  if (mfEnergy.first / mfEnergy.second >= mEvoData.mfReplicationThresh)
    Replicate();

  return v;
}

void Ecosystem::Creature::Replicate(void) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<float> dist(0.f, 1.f);
  std::uniform_real_distribution<float> distMutV(-CREATURE_MUTATION_EPSILON,
                                                 CREATURE_MUTATION_EPSILON);

  if (dist(mt) <= mEvoData.mfReplicateChance) {
    GridPos p = EcoSystem::GetInst().GetEmptyNeighbour(GetGridPosition());
    if (p.x < 0 || p.y < 0) return;

    float sze = mTraits.mfSize;
    float spd = mTraits.mfSpeed;
    float sen = mTraits.mfSense;

    if (dist(mt) <= mEvoData.mfMutationChance) {
      sze = Clamp(0.01f, 100.f, sze + distMutV(mt));
      spd = Clamp(0.01f, 100.f, spd + distMutV(mt));
      spd = Clamp(0.01f, 100.f, spd + distMutV(mt));
      sen = Clamp(0.01f, 100.f, sen + distMutV(mt));
    }

    ConsumeEnergy(::GetActionCost(mTraits.mfSize, mTraits.mfSpeed,
                                  mTraits.mfSense, mfEnergy.first,
                                  ::Action::REPLICATE));
    Data::VisitSpawnTuple(
        Data::SpawnVisitor{p.x, p.y, EvolutionChart[mnChartID],
                           Traits{sze, spd, sen}},
        mnChartID);
  }
}

void Ecosystem::Creature::UpdateAwake(float _dt) noexcept {
  if (_dt <= 0.f) return;

  ConsumeEnergy(::GetActionCost(mTraits.mfSize, mTraits.mfSpeed,
                                mTraits.mfSense, mfEnergy.first, ::Action::IDLE,
                                _dt));
  if (mfEnergy.first <= 0.f) mBitFlags |= Flags::FLAG_DEAD;

  if (!mCurPath.empty()) Move(_dt);

  this->UpdateAwakeBehaviour(_dt);
}

void Ecosystem::Creature::UpdateAsleep(float _dt) noexcept {
  this->UpdateAsleepBehaviour(_dt);
}
