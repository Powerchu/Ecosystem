#include "EcoSystem/EcoSystem.h"
#include "Data/EcoData.h"

#include <GLFW/glfw3.h>
#include <algorithm>

#include "imgui.h"
#include "imgui_internal.h"

#define FIXED_DT 0.01666666666f

namespace CS380
{
	extern EvolutionData EvolutionChart[EVOLUTION_CHART_COUNT];
	extern float CREATURE_MUTATION_EPSILON;
}

template<typename T>
T min(T l, T r)
{
	return l < r ? l : r;
}

CS380::EcoSystem& CS380::EcoSystem::GetInst(void) noexcept
{
	static CS380::EcoSystem eco{ 64, 64, 32 };
	return eco;
}

CS380::EcoSystem::EcoSystem(unsigned _w, unsigned _h, unsigned _s) noexcept
	: mnLogWindow{ 20 }, mnWidth{ _w }, mnHeight{ _h }, mnScale{ _s }, mnWindowX{ 0 }, mnWindowY{ 0 }, mfDelta{ 0.016f },
	mfTimeStep{ 1.f }, mbEcoTool{ true }, mbRunEco{ false }, mfTitleBarSize{ 0.f }, mTerrain{ mnWidth, mnHeight },
	mAllCreatures{}, mTools{}, mHighlightQueue{}, mLogs{}, mfScalar{}, mfLogFreq{ 1.f }, mfLogAccDt{ 0.f }, mnPeakPops{ 0 },
	mfInitialGrassA{ 0.1f },
	mfInitialGrassVLo{ 0.025f },
	mfInitialGrassVHi{ 1.0f },
	mfGrassMaxEnergy{ 300.f },
	mfFertilizerMaxEnergy{ 1000.f },
	mfDeathThresh{ 0.3f },
	mfGRateLo{ 0.0001f },
	mfGRateHi{ 0.05f },
	mfFRateLo{ 0.00001f },
	mfFRateHi{ 0.00115f }
{
	mLogs.resize(LogTypes::LAST);
	for (auto& l : mLogs)
		l.resize(mnLogWindow, 0.f);
}

void CS380::EcoSystem::Init(void) noexcept
{
	Data::MakeTools();
}

CS380::EcoSystem::~EcoSystem(void) noexcept
{
	for (auto& c : mAllCreatures)
		delete c;
	for (auto& t : mTools)
		delete t;
}

void CS380::EcoSystem::UpdateWindowSize(int _x, int _y) noexcept
{
	mnWindowX = _x; 
	mnWindowY = _y;
}

void CS380::EcoSystem::Update(float _dt) noexcept
{
	mfDelta = (_dt * mfTimeStep);

	if (mbRunEco)
	{
		// pre
		mTerrain.Update(mfDelta);
		UpdateCreatures(mfDelta);


		// post
		CleanUpDead();
		UpdateMap();

		if (mbEcoTool)
			EcoTool();
		UpdateTools();

		mfLogAccDt += mfDelta;
		if (mfLogAccDt > 1.f / mfLogFreq)
			UpdateLogs();

		RenderMap();
	}
	else
	{
		RenderSetup();
	}
}

void CS380::EcoSystem::AddCreature(CS380::Creature* _c)
{
	mAllCreatures.push_back(_c);
	mnPeakPops++;
}

void CS380::EcoSystem::RenderMap(void)
{
	ImGui::SetNextWindowPos(ImVec2{ 0.f,0.f }, ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2{ mnWindowX, mnWindowY }, ImGuiCond_Always);
	ImGui::Begin("Simulation Space", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar );

	RenderMenuBar();
	RenderGrid();
	RenderGridOverlay();
	RenderHighlights();

	ImGui::End();
}

void CS380::EcoSystem::RenderGrid(void)
{
	ImRect bounds{ ImVec2{ 6.f, mfTitleBarSize + 6.f }, ImVec2{ static_cast<float>(mnWindowX) - 6.f, static_cast<float>(mnWindowY) - 6.f} };

	ImVec2 space{ bounds.Max.x - bounds.Min.x, bounds.Max.y - bounds.Min.y };
	space.x /= static_cast<float>(mnWidth);
	space.y /= static_cast<float>(mnHeight);
	mfScalar = min(min(space.x, space.y), static_cast<float>(mnScale));
	ImVec2 box{ bounds.Min.x + static_cast<float>(mfScalar), bounds.Min.y + static_cast<float>(mfScalar) };

	ImDrawList *pDrawList = ImGui::GetWindowDrawList();
	for (unsigned y = 0; y < mnHeight; ++y)
	{
		for (unsigned x = 0; x < mnWidth; ++x)
		{
			ImVec2 min{ bounds.Min.x + (x * mfScalar), bounds.Min.y + (y * mfScalar) };
			ImVec2 max{ box.x + (x * mfScalar), box.y + (y * mfScalar) };

			// has creature on it
			if (mTerrain.GetSpaceLayer()[y][x] != -1)
			{
				pDrawList->AddRectFilled(min, max, mAllCreatures[mTerrain.GetSpaceLayer()[y][x]]->GetColor());
			}
			// no creature on it
			else
			{
				pDrawList->AddRectFilled(min, max, mTerrain.GetGrassColor(x,y));
			}
		}
	}
}

void CS380::EcoSystem::RenderGridOverlay(void) noexcept
{
	ImRect bounds{ ImVec2{ 6.f, mfTitleBarSize + 6.f }, ImVec2{ static_cast<float>(mnWindowX) - 6.f, static_cast<float>(mnWindowY) - 6.f} };
	ImVec2 box{ bounds.Min.x + static_cast<float>(mfScalar), bounds.Min.y + static_cast<float>(mfScalar) };

	ImDrawList *pDrawList = ImGui::GetWindowDrawList();
	for (unsigned y = 0; y < mnHeight; ++y)
	{
		for (unsigned x = 0; x < mnWidth; ++x)
		{
			ImVec2 min{ bounds.Min.x + (x * mfScalar), bounds.Min.y + (y * mfScalar) };
			ImVec2 max{ box.x + (x * mfScalar), box.y + (y * mfScalar) };
			pDrawList->AddRect(min, max, ImGui::GetColorU32(ImVec4{0.5f,0.5f,0.5f, 0.1f}));
		}
	}
}

void CS380::EcoSystem::RenderHighlights(void)
{
	ImRect bounds{ ImVec2{ 6.f, mfTitleBarSize + 6.f }, ImVec2{ static_cast<float>(mnWindowX) - 6.f, static_cast<float>(mnWindowY) - 6.f} };
	ImVec2 box{ bounds.Min.x + static_cast<float>(mfScalar), bounds.Min.y + static_cast<float>(mfScalar) };
	ImDrawList *pDrawList = ImGui::GetWindowDrawList();
	while (!mHighlightQueue.empty())
	{
		std::tuple<unsigned, unsigned, unsigned int> h = mHighlightQueue.top();
		mHighlightQueue.pop();

		ImVec2 min{ bounds.Min.x + (std::get<0>(h) * mfScalar), bounds.Min.y + (std::get<1>(h) * mfScalar) };
		ImVec2 max{ box.x + (std::get<0>(h) * mfScalar), box.y + (std::get<1>(h) * mfScalar) };
		pDrawList->AddRect(min, max, std::get<2>(h), 0.f, 15, 3.f);
	}
}

void CS380::EcoSystem::RenderMenuBar(void)
{
	ImGui::BeginMainMenuBar();
	mfTitleBarSize = ImGui::GetWindowSize().y;
	if (ImGui::BeginMenu("Windows"))
	{
		ImGui::PushID(99);
		ImGui::Selectable("EcoSystem", &mbEcoTool);
		ImGui::PopID();

		for (unsigned i = 0; i < mTools.size(); ++i)
		{
			ImGui::PushID(static_cast<int>(i));
			ImGui::Selectable(mTools[i]->GetName().c_str(), mTools[i]->GetOpened());
			ImGui::PopID();
		}
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void CS380::EcoSystem::EcoTool(void)
{
	int v = static_cast<int>(mnLogWindow);
	ImGui::Begin("EcoSystem", &mbEcoTool);
	ImGui::DragFloat("Time step", &mfTimeStep, 0.1f, 0.f, 100.f);

	if (ImGui::CollapsingHeader("Evolution Models"))
	{
		ImGui::DragFloat("Mutation Epsilon: %f", &CREATURE_MUTATION_EPSILON, 0.01f, 0.001f, 0.999f);
		for (int i = 0; i < EVOLUTION_CHART_COUNT; ++i)
		{
			ImGui::PushID(i);
			if (ImGui::CollapsingHeader(Spawnables[i]))
			{
				ImGui::DragFloat("Replication Thresh: %f", &EvolutionChart[i].mfReplicationThresh, 0.01f, 0.001f, 0.999f);
				ImGui::DragFloat("Replication Chance: %f", &EvolutionChart[i].mfReplicateChance, 0.01f, 0.001f, 0.999f);
				ImGui::DragFloat("Mutation Chance: %f", &EvolutionChart[i].mfMutationChance, 0.01f, 0.001f, 0.999f);
			}
			ImGui::PopID();
		}
	}

	ImGui::DragInt("Log Window", &v, 1.f, 20, 100);
	ImGui::DragFloat("Log Freq.", &mfLogFreq, 0.1f, 1.f, 100.f);
	mnLogWindow = static_cast<unsigned>(v);
	if (ImGui::Button("Nuke", ImVec2{100.f,20.f}))
		Nuke();
	ImGui::End();
}

void CS380::EcoSystem::UpdateMap(void)
{
	unsigned x, y;
	for (auto& row : mTerrain.GetSpaceLayer())
		for (auto& col : row)
			col = -1;

	for (int i = 0; i < static_cast<int>(mAllCreatures.size()); ++i)
	{
		mAllCreatures[i]->GetGridPosition(x, y);
		mTerrain.GetSpaceLayer()[y][x] = i;
	}
}

void CS380::EcoSystem::UpdateTools(void)
{
	for (unsigned i = 0; i < mTools.size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		if (*mTools[i]->GetOpened())
			mTools[i]->Render();
		ImGui::PopID();
	}
}

void CS380::EcoSystem::UpdateCreatures(float _dt) const
{
	for (auto& c : mAllCreatures)
	{
		if (c->GetFlags() & CS380::Creature::FLAG_DEAD)
			continue;

		c->UpdateAwake(_dt);
	}
}

void CS380::EcoSystem::CleanUpDead(void)
{
	for (unsigned i = 0; i < mAllCreatures.size(); ++i)
	{
		if (mAllCreatures[i]->GetFlags() & CS380::Creature::FLAG_DEAD)
		{
			auto p = mAllCreatures[i]->GetGridPosition();

			// check if he died with somebody else on it or naturally 
			if (mTerrain.GetSpaceLayer()[p.y][p.x] == static_cast<int>(i))
				mTerrain.GetSpaceLayer()[p.y][p.x] = -1;

			mTerrain.GetFertilizerLayer()[p.y][p.x] += mAllCreatures[i]->GetEnergy().second * mfDeathThresh;

			std::swap(mAllCreatures[i], mAllCreatures.back());
			Creature *c = mAllCreatures.back();
			mAllCreatures.pop_back();
			delete c;
			--i;
		}
	}
}

void CS380::EcoSystem::HighlightGrid(unsigned short x, unsigned short y, unsigned int _col)
{
	mHighlightQueue.push(std::make_tuple(x, y, _col));
}

void CS380::EcoSystem::HighlightGrid(unsigned x, unsigned y, unsigned int _col)
{
	mHighlightQueue.push(std::make_tuple(static_cast<unsigned short>(x), static_cast<unsigned short>(y), _col));
}

int CS380::EcoSystem::GetWidth(void) const noexcept
{
	return mnWidth;
}

int CS380::EcoSystem::GetHeight(void) const noexcept
{
	return mnHeight;
}

int CS380::EcoSystem::GetGridVal(unsigned _x, unsigned _y) const noexcept
{
	if (_x >= mnWidth || _y >= mnHeight)
		return -1;


	auto v = mTerrain.GetSpaceLayer()[_y][_x];
	if (v>= 0 && v >= mAllCreatures.size())
		__debugbreak(); // out of range? layer not updated? execution order?
	return v;
}

float CS380::EcoSystem::GetGrassVal(unsigned _x, unsigned _y) const noexcept
{
	if (_x > mnWidth || _y > mnHeight)
		return 0.f;

	return mTerrain.GetGrassLayer()[_y][_x];
}

float CS380::EcoSystem::GetGrassValA(unsigned _x, unsigned _y) const noexcept
{
	if (_x > mnWidth || _y > mnHeight)
		return 0.f;

	return mTerrain.GetGrassLayer()[_y][_x] / mTerrain.GetGrassLayerThresh()[_y][_x].second;
}

const CS380::Terrain& CS380::EcoSystem::GetTerrain(void) const noexcept
{
	return mTerrain;
}

CS380::Creature * CS380::EcoSystem::GetCreature(int i)
{
	if (i < 0 || i >= mAllCreatures.size())
		return nullptr;

	return mAllCreatures[i];
}

const std::deque<CS380::Creature *>& CS380::EcoSystem::GetAllCreatures(void) const noexcept
{
	return mAllCreatures;
}

void CS380::EcoSystem::Nuke(void) noexcept
{
	for (auto& c : mAllCreatures)
	{
		ReturnEnergyToMap(c->ConsumeEnergy(c->GetEnergy().second), c->GetGridPosition());
		//c->ConsumeFatigue(FLT_MAX);
	}
}

float CS380::EcoSystem::Eat(const GridPos& _p, Creature* _predator)
{
	unsigned x, y;
	_predator->GetGridPosition(x,y);

	if (sqrt((_p.x - static_cast<int>(x)) * (_p.x - static_cast<int>(x)) + (_p.y - static_cast<int>(y)) * (_p.y - static_cast<int>(y))) > 1.5f)
		__debugbreak(); // attempting to eat from further than 1 unit away??

	auto i = mTerrain.GetSpaceLayer()[_p.y][_p.x];
	// got other creature, means eating it ?
	if (i >= 0 && i < mAllCreatures.size())
	{
		if (mAllCreatures[i] == _predator)
		{
			if (dynamic_cast<Fox*>(_predator))
			{
				return 0;
			}
			// eating yourself? assume eat grass you're on
			return mTerrain.ConsumeGrass(_p.x, _p.y, 1.f);
		}
		if (const auto fox = dynamic_cast<Fox*>(mAllCreatures[i]))
		{
			// rabbit cant eat foxes
			if (fox && dynamic_cast<Rabbit*>(_predator))
			{
				return mTerrain.ConsumeGrass(_p.x, _p.y, 1.f);
			}
			
		}
		if (mAllCreatures[i]->GetSize() < 1.2f * _predator->GetSize())
		{
			return mAllCreatures[i]->Eaten(_predator);
		}
	}
	else
		mTerrain.GetSpaceLayer()[_p.y][_p.x] = -1;

	// no creature, means eating grass?
	return mTerrain.ConsumeGrass(_p.x, _p.y, 1.f);
}

std::vector<CS380::GridPos> CS380::EcoSystem::GetShortestPath(const GridPos& _src, const GridPos& _dest)
{
	return mTerrain.GetShortestPath(_src, _dest);
}

CS380::GridPos CS380::EcoSystem::GetBestGrassPos(const GridPos& _src, float _radiusLimit, float _minAlpha)
{
	return mTerrain.GetBestGrassPos(_src, _radiusLimit, _minAlpha);
}

CS380::GridPos CS380::EcoSystem::GetEmptyNeighbour(const GridPos& _src)
{
	return mTerrain.GetEmptyNeighbour(_src);
}

std::pair<float, float> CS380::EcoSystem::GetScreenPos(const GridPos& _p) const noexcept
{
	ImRect bounds{ ImVec2{ 6.f, mfTitleBarSize + 6.f }, ImVec2{ static_cast<float>(mnWindowX) - 6.f, static_cast<float>(mnWindowY) - 6.f} };

	ImVec2 space{ bounds.Max.x - bounds.Min.x, bounds.Max.y - bounds.Min.y };
	space.x /= static_cast<float>(mnWidth);
	space.y /= static_cast<float>(mnHeight);
	float scale = min(min(space.x, space.y), static_cast<float>(mnScale));
	ImVec2 box{ bounds.Min.x + static_cast<float>(scale), bounds.Min.y + static_cast<float>(scale) };

	ImVec2 min{ bounds.Min.x + (_p.x * scale), bounds.Min.y + (_p.y * scale) };
	ImVec2 max{ box.x + (_p.x * scale), box.y + (_p.y * scale) };
	return std::make_pair((min.x + max.x) / 2, (min.y + max.y) / 2);
}

float CS380::EcoSystem::GetScalar(void) const noexcept
{
	return mfScalar;
}

const std::vector<std::deque<float>>& CS380::EcoSystem::GetLogs(void) const noexcept
{
	return mLogs;
}

void CS380::EcoSystem::ReturnEnergyToMap(float _v, const GridPos& _p) noexcept
{
	mTerrain.GetFertilizerLayer()[_p.y][_p.x] += _v;
}

void CS380::EcoSystem::UpdateLogs(void) noexcept
{
	mfLogAccDt = 0.f;
	float v[LogTypes::LAST] = { 0.f, 0.f, 0.f, 0.f, 0.f };
	for (const auto& c : mAllCreatures)
	{
		v[0] += c->GetSpeed();
		v[1] += c->GetSize();
		v[2] += c->GetSense();
	}

	v[3] = static_cast<float>(mAllCreatures.size());
	const auto& g = mTerrain.GetGrassLayer();
	const auto& l = mTerrain.GetGrassLayerThresh();
	for (unsigned y = 0; y < g.size(); ++y)
		for (unsigned x = 0; x < g[y].size(); ++x)
			v[4] += (g[y][x] / l[y][x].second);

	for (int i = 0; i <= LogTypes::AVG_SENSE; ++i)
	{
		if (mLogs[i].size() == mnLogWindow)
			mLogs[i].pop_front();

		if (mAllCreatures.size())
			mLogs[i].push_back(v[i] / static_cast<float>(mAllCreatures.size()));
		else
			mLogs[i].push_back(0.f);
	}

	for (int i = LogTypes::CREATURES_COUNTER; i < LogTypes::LAST; ++i)
	{
		if (mLogs[i].size() == mnLogWindow)
			mLogs[i].pop_front();
		mLogs[i].push_back(v[i]);
	}
}

void CS380::EcoSystem::RenderSetup(void) noexcept
{
	ImGui::Begin("Set up Simulation");

	int w = static_cast<int>(mnWidth);
	int h = static_cast<int>(mnHeight);

	if (ImGui::DragInt("Width", &w, 1.f, 10, 200))
	{
		w = w < 10 ? 10 : w > 200 ? 200 : w;
		mnWidth = static_cast<unsigned>(w);
	}
	if (ImGui::DragInt("Height", &h, 1.f, 10, 200))
	{
		h = h < 10 ? 10 : h > 200 ? 200 : h;
		mnHeight = static_cast<unsigned>(h);
	}

	ImGui::DragFloat("Random Grass A", &mfInitialGrassA, 0.1f, 0.f, 1.f);
	if (ImGui::DragFloat("Grass V Lo", &mfInitialGrassVLo, 0.1f, 0.f, mfInitialGrassVHi))
		mfInitialGrassVHi = mfInitialGrassVHi < mfInitialGrassVLo ? mfInitialGrassVLo : mfInitialGrassVHi;
	if (ImGui::DragFloat("Grass V Hi", &mfInitialGrassVHi, 0.1f, mfInitialGrassVLo, 1.f))
		mfInitialGrassVHi = mfInitialGrassVHi < mfInitialGrassVLo ? mfInitialGrassVLo : mfInitialGrassVHi;

	if (ImGui::DragFloat("Grass G Lo", &mfGRateLo, 0.001f, 0.f, mfGRateHi))
		mfGRateHi = mfGRateHi < mfGRateLo ? mfGRateLo : mfGRateHi;
	if (ImGui::DragFloat("Grass G Hi", &mfGRateHi, 0.001f, mfGRateLo, 0.1f))
		mfGRateHi = mfGRateHi < mfGRateLo ? mfGRateLo : mfGRateHi;

	ImGui::DragFloat("Grass Max E", &mfGrassMaxEnergy, 0.1f, 0.f, 10000.f);
	if (ImGui::DragFloat("Fert G Lo", &mfFRateLo, 0.001f, 0.f, mfFRateHi))
		mfFRateHi = mfFRateHi < mfFRateLo ? mfFRateLo : mfFRateHi;
	if (ImGui::DragFloat("Fert G Hi", &mfFRateHi, 0.001f, mfFRateLo, 0.01f))
		mfFRateHi = mfFRateHi < mfFRateLo ? mfFRateLo : mfFRateHi;
	ImGui::DragFloat("Fert Max E", &mfFertilizerMaxEnergy, 0.1f, 0.f, 10000.f);
	ImGui::DragFloat("Death Threshhold", &mfDeathThresh, 0.1f, 0.01f, 1.);
	if (ImGui::Button("Begin!", ImVec2{ 120.f, 30.f }))
	{
		mTerrain.Init(mfInitialGrassA, mfInitialGrassVLo, mfInitialGrassVHi, mnWidth, mnHeight, mfGRateLo, mfGRateHi, mfGrassMaxEnergy, mfFertilizerMaxEnergy, mfFRateLo, mfFRateHi);
		mbRunEco = true;
	}

	ImGui::End();
}









