#include "EcoSystem/Tools/ViewerTool.h"
#include "Data/EcoData.h"

#include "imgui.h"
#include "imgui_internal.h"

Ecosystem::ViewTool::ViewTool(bool _open) noexcept
	: Tools{ "View Tool", _open }, mnCurrSelection{ -1 }
{
}

Ecosystem::ViewTool::~ViewTool(void) noexcept
{}

void Ecosystem::ViewTool::Render(void) noexcept
{
	const auto& cs = EcoSystem::GetInst().GetAllCreatures();
	static float constexpr indent = 10.f;
	ImGui::Begin(mName.c_str(), &mbOpened);
	ImGui::BeginColumns("View", 2);

	for (unsigned i = 0; i < cs.size(); ++i)
	{
		ImGui::TextDisabled("%d) %s", i, cs[i]->GetUniqueID().c_str());
		if (ImGui::IsItemHovered())
		{
			unsigned x, y;
			cs[i]->GetGridPosition(x, y);
			EcoSystem::GetInst().HighlightGrid(x, y, ImGui::GetColorU32(ImVec4{ 1.f,1.f,1.f,1.f }));

			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::Text("General");
			ImGui::Indent(indent);
			ImGui::Text("UID  : %s", cs[i]->GetUniqueID().c_str());
			ImGui::Text("Name : %s", cs[i]->GetName().c_str());
			ImGui::Text("Mass : %f / %f", cs[i]->GetEnergy().first, cs[i]->GetEnergy().second);
			ImGui::Unindent(indent);
			ImGui::Text("Traits");
			ImGui::Indent(indent);
			ImGui::Text("Size : %f", cs[i]->GetSize());
			ImGui::Text("Speed: %f", cs[i]->GetSpeed());
			ImGui::Text("Sense: %f", cs[i]->GetSense());
			ImGui::Unindent(indent);
			ImGui::Text("Evolution");
			ImGui::Indent(indent);
			ImGui::Text("Rep. : %f", cs[i]->GetRepChance());
			ImGui::Text("Mut. : %f", cs[i]->GetMutChance());
			ImGui::Unindent(indent);

			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	ImGui::NextColumn();

	const auto& terrain = EcoSystem::GetInst().GetTerrain();
	const auto& grid = terrain.GetGrassLayer();
	for (unsigned y = 0; y < grid.size(); ++y)
	{
		for (unsigned x = 0; x < grid[y].size(); ++x)
		{
			ImGui::TextDisabled("Grid [%d][%d]", x, y);
			if (ImGui::IsItemHovered())
			{
				EcoSystem::GetInst().HighlightGrid(x, y, ImGui::GetColorU32(ImVec4{ 1.f,1.f,1.f,1.f }));

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::Text("Pos x,y: %d, %d", x, y);
				ImGui::Text("Grass");
				ImGui::Indent(indent);
				ImGui::Text("Val    : %f", terrain.GetGrassLayer()[y][x]);
				ImGui::Text("Rate   : %f", terrain.GetGrassLayerRate()[y][x]);
				ImGui::Text("Thresh : %f / %f", terrain.GetGrassLayerThresh()[y][x].first, terrain.GetGrassLayerThresh()[y][x].second);
				ImGui::Unindent(indent);
				ImGui::Text("Fertilizer");
				ImGui::Indent(indent);
				ImGui::Text("Val    : %f", terrain.GetFertilizerLayer()[y][x]);
				ImGui::Text("Thresh : %f / %f", terrain.GetFertilizerLayerThresh()[y][x].first, terrain.GetFertilizerLayerThresh()[y][x].second);
				ImGui::Unindent(indent);
				ImGui::Text("Occupancy");
				ImGui::Indent(indent);
				ImGui::Text("Indx   : %d", terrain.GetSpaceLayer()[y][x]);
				ImGui::Unindent(indent);

				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
	}

	ImGui::EndColumns();
	ImGui::End();
}

