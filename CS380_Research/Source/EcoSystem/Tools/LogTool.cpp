#include "EcoSystem/Tools/LogTool.h"
#include "EcoSystem/EcoSystem.h"
#include "imgui.h"

CS380::LogTool::LogTool(bool _opened) noexcept
	: Tools{ "Logging", _opened }
{
}

CS380::LogTool::~LogTool(void) noexcept
{
}

void CS380::LogTool::Render(void) noexcept
{
	auto& eco = EcoSystem::GetInst();
	auto& logs = eco.GetLogs();
	ImGui::Begin(mName.c_str(), &mbOpened);
	std::vector<float> res;
	for (unsigned i = 0; i < logs.size(); ++i)
	{
		res.resize(logs[i].size(), 0);
		for (unsigned j = 0; j < res.size(); ++j)
			res[j] = logs[i][j];

		switch (i)
		{
		case 0:
			ImGui::PlotHistogram("Average Speed", res.data(), static_cast<int>(res.size()), 0, NULL, 0.0f, 2.f, ImVec2(0, 30));
			break;
		case 1:
			ImGui::PlotHistogram("Average Size", res.data(), static_cast<int>(res.size()), 0, NULL, 0.0f, 2.f, ImVec2(0, 30));
			break;
		case 2:
			ImGui::PlotHistogram("Average Sense", res.data(), static_cast<int>(res.size()), 0, NULL, 0.0f, 2.f, ImVec2(0, 30));
			break;
		case 3:
			ImGui::PlotHistogram("Creature Population", res.data(), static_cast<int>(res.size()), 0, NULL, 0.0f, static_cast<float>(eco.mnPeakPops), ImVec2(0, 80));
			break;
		case 4:
			ImGui::PlotHistogram("Grass Density", res.data(), static_cast<int>(res.size()), 0, NULL, 0.0f, static_cast<float>(eco.GetWidth() * eco.GetHeight()), ImVec2(0, 80));
			break;
		}
	}
	ImGui::End();
}




