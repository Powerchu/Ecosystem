#ifndef _ECOSYSTEM_H_
#define _ECOSYSTEM_H_

#include "Terrain.h"
#include "Tools/Tools.h"

#include <stack>
#include <tuple>
#include <vector>

namespace Ecosystem {
class Creature;
class Tools;
enum LogTypes {
  AVG_SPEED,
  AVG_SIZE,
  AVG_SENSE,
  CREATURES_COUNTER,
  GRASS_COUNTER,
  LAST
};

class EcoSystem {
 public:
  static EcoSystem& GetInst(void) noexcept;
  ~EcoSystem(void) noexcept;

  void Init(void) noexcept;

  void UpdateWindowSize(int, int) noexcept;
  void Update(float) noexcept;

  // aux inits
  void AddCreature(Creature*);
  template <typename T,
            typename SFNAE = std::enable_if_t<std::is_base_of_v<Tools, T>, T>>
  void AddTools(T* _pTool);

  // aux visual aid
  void HighlightGrid(unsigned short x, unsigned short y, unsigned int _col);
  void HighlightGrid(unsigned x, unsigned y, unsigned int _col);

  // getters
  int GetWidth(void) const noexcept;
  int GetHeight(void) const noexcept;
  int GetGridVal(unsigned _x, unsigned _y) const noexcept;
  float GetGrassVal(unsigned _x, unsigned _y) const noexcept;
  float GetGrassValA(unsigned _x, unsigned _y) const noexcept;
  const Terrain& GetTerrain(void) const noexcept;
  Creature* GetCreature(int i);
  const std::deque<Creature*>& GetAllCreatures(void) const noexcept;
  std::vector<GridPos> GetShortestPath(const GridPos& _src,
                                       const GridPos& _dest);
  GridPos GetBestGrassPos(const GridPos& _src, float _radiusLimit,
                          float _minAlpha);
  GridPos GetEmptyNeighbour(const GridPos& _src);
  std::pair<float, float> GetScreenPos(const GridPos& _p) const noexcept;
  float GetScalar(void) const noexcept;
  const std::vector<std::deque<float>>& GetLogs(void) const noexcept;

  // fun functions
  void Nuke(void) noexcept;
  void ReturnEnergyToMap(float _v, const GridPos& _p) noexcept;

  // returns 0-1 if its grass, returns size prey's remainding energy, predator
  // automatically gains energy according to return val
  float Eat(const GridPos& _p, Creature* _predator);

  unsigned mnPeakPops;

 protected:
  EcoSystem(unsigned _w = 0, unsigned _h = 0, unsigned _s = 1) noexcept;

  unsigned mnLogWindow;
  unsigned mnWidth;
  unsigned mnHeight;
  unsigned mnScale;

  int mnWindowX;
  int mnWindowY;
  float mfDelta;
  float mfTitleBarSize;
  float mfTimeStep;
  float mfLogFreq;
  float mfLogAccDt;
  float mfScalar;
  bool mbEcoTool;
  bool mbRunEco;

  Terrain mTerrain;

  std::deque<Creature*> mAllCreatures;
  std::vector<Tools*> mTools;
  std::stack<std::tuple<unsigned short, unsigned short, unsigned int>>
      mHighlightQueue;

  std::vector<std::deque<float>> mLogs;

  float mfInitialGrassA;
  float mfInitialGrassVLo;
  float mfInitialGrassVHi;
  float mfGrassMaxEnergy;
  float mfFertilizerMaxEnergy;
  float mfDeathThresh;

  float mfGRateLo;
  float mfGRateHi;
  float mfFRateLo;
  float mfFRateHi;

  void RenderMap(void);
  void RenderGrid(void);
  void RenderGridOverlay(void) noexcept;
  void RenderHighlights(void);
  void RenderMenuBar(void);
  void RenderSetup(void) noexcept;
  void UpdateTools(void);
  void UpdateMap(void);
  void UpdateCreatures(float) const;
  void UpdateLogs(void) noexcept;
  void EcoTool(void);
  void CleanUpDead(void);

 private:
};

template <typename T, typename SFNAE>
inline void EcoSystem::AddTools(T* _pTool) {
  mTools.push_back(static_cast<Tools*>(_pTool));
}
}  // namespace Ecosystem

#endif
