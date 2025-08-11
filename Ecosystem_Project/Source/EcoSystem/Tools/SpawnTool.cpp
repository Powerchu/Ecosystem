#include "EcoSystem/Tools/SpawnTool.h"
#include "Data/EcoData.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <random>

namespace Ecosystem
{
	extern EvolutionData EvolutionChart[EVOLUTION_CHART_COUNT];
}

Ecosystem::SpawnTool::SpawnTool(bool _opened) noexcept
	: Tools{ "SpawnTool", _opened }, mnSpawnX{ 0 }, mnSpawnY{ 0 }, mnCurrSelection{ 0 }, mnSpawnCount{ 1 },
	mfCurSize{ 1.f }, mfCurSpeed{ 1.f }, mfCurSense{ 1.f }
{
}

Ecosystem::SpawnTool::~SpawnTool(void) noexcept
{
}

void Ecosystem::SpawnTool::Render(void) noexcept
{
	EcoSystem& eco = EcoSystem::GetInst();
	ImGui::Begin(mName.c_str(), &mbOpened);
	ImGui::Combo("Creatures", &mnCurrSelection, Spawnables, CREATURE_COUNT);
	ImGui::DragFloat("Size ", &mfCurSize, 0.1f, 1.f, MAX_CREATURE_SIZE);
	ImGui::DragFloat("Speed ", &mfCurSpeed, 0.1f, 1.f, MAX_CREATURE_SPEED);
	ImGui::DragFloat("Sense ", &mfCurSense, 0.1f, 1.f, MAX_CREATURE_SENSE);
	if (ImGui::CollapsingHeader("Singular"))
	{
		ImGui::DragInt("X ", &mnSpawnX, 1.f, 0, eco.GetWidth() - 1);
		ImGui::DragInt("Y ", &mnSpawnY, 1.f, 0, eco.GetHeight() - 1);
		unsigned short x = static_cast<unsigned short>(mnSpawnX);
		unsigned short y = static_cast<unsigned short>(mnSpawnY);
		if (ImGui::ButtonEx("Spawn", ImVec2{ 80, 30 }, (eco.GetGridVal(x, y) == -1 ? 0 : ImGuiButtonFlags_Disabled)))
		{
			Data::VisitSpawnTuple(
				Data::SpawnVisitor{ x, y, EvolutionChart[mnCurrSelection], Traits{ mfCurSize, mfCurSpeed, mfCurSense } }
			, mnCurrSelection);
		}
		eco.HighlightGrid(x, y, ImGui::GetColorU32({ 1.f,0.f,0.f,0.5f }));
	}
	
	if (ImGui::CollapsingHeader("Multiple"))
	{
		ImGui::DragInt("Count ", &mnSpawnCount, 1.f, 1, 100);
		if (ImGui::ButtonEx("Batch Spawn", ImVec2{ 80, 30 }))
		{
			std::random_device rd;
			std::mt19937 mt(rd());
			for (int i = 0; i < mnSpawnCount; ++i)
			{
				std::uniform_int_distribution<unsigned short> distX(0, static_cast<unsigned short>(EcoSystem::GetInst().GetWidth()) - 1);
				std::uniform_int_distribution<unsigned short> distY(0, static_cast<unsigned short>(EcoSystem::GetInst().GetHeight()) - 1);
				unsigned short x = distX(mt);
				unsigned short y = distY(mt);
				int failsafe = 10000;
				while (eco.GetGridVal(x, y) != -1)
				{
					x = distX(mt);
					y = distY(mt);
					failsafe--;
					if (failsafe < 0)
						break;
				}
				if (failsafe > 0)
				{
					auto a = EvolutionChart[mnCurrSelection];
					Data::VisitSpawnTuple(
						Data::SpawnVisitor{ x, y, EvolutionChart[mnCurrSelection], Traits{ mfCurSize, mfCurSpeed, mfCurSense } }
					, mnCurrSelection);
				}
			}
		}
	}

	ImGui::End();
}









