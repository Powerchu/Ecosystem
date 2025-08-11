#include "Data/EcoData.h"
#include "EcoSystem/EcoSystem.h"
#include "EcoSystem/Tools/LogTool.h"
#include "EcoSystem/Tools/SpawnTool.h"
#include "EcoSystem/Tools/ViewerTool.h"

Ecosystem::EvolutionData Ecosystem::EvolutionChart[EVOLUTION_CHART_COUNT] = {
    Ecosystem::EvolutionData{0.7f, 0.001f,
                             0.75f},  // indicate your new user defined rate
    Ecosystem::EvolutionData{0.35f, 0.30f, 0.6667f}  // hard coded presets
};

void Ecosystem::Data::MakeTools(void) {
  EcoSystem& eco = EcoSystem::GetInst();

  eco.AddTools(new SpawnTool{});
  eco.AddTools(new ViewTool{});
  eco.AddTools(new LogTool{});
}
