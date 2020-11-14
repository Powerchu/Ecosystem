#include "EcoSystem/Terrain.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <queue>
#include <random>
#include <utility>

#define SQRT_2 1.41421356237f

struct Comp
{
	template <typename T>
	bool operator()(T* _a, T* _b)
	{
		return *_a < *_b;
	}
};

template <typename T>
T Max(const T& _a, const T& _b)
{
	return _a > _b ? _a : _b;
}

template <typename T>
T Min(const T& _a, const T& _b)
{
	return _a < _b ? _a : _b;
}

template<typename T>
T Clamp(T min, T max, T val)
{
	return val > max ? max : val < min ? min : val;
}

template <typename T>
T Lerp(T _from, T _to, float _a)
{
	return _from + (_to - _from) * _a;
}

float GetOctileCost(float x, float y)
{
	return Max(x, y) + (SQRT_2 - 1)*Min(x, y);
}

CS380::GridPos::GridPos(int _x, int _y) noexcept
	: x{ _x }, y{ _y }
{}

bool CS380::GridPos::operator==(const GridPos& _rhs) const
{
	return x == _rhs.x && y == _rhs.y;
}

CS380::Node::Node(void) noexcept
	: pos{ 0,0 }, tcost{ 0 }, hcost{ 0 }, fcost{ 0.f }, mpPrev{ nullptr }
{}

CS380::Node::Node(const GridPos& _p, float _t, float _h) noexcept
	: pos{ _p }, tcost{ _t }, hcost{ _h }, fcost{ 0.f }, mpPrev{ nullptr }
{}

bool CS380::Node::operator<(const Node& _rhs) const noexcept
{
	return fcost > _rhs.fcost;
}

CS380::Terrain::Terrain(unsigned _x, unsigned _y)
	: mnWidth{ _x }, mnHeight{ _y }, mSpaceLayer{}, 
	mGrassLayer{}, mFertilizerLayer{}, 
	mGrassLayerRate{},
	mGrassThresh{}, mFertilizerThresh{}
{
}

void CS380::Terrain::Init(float _iga, float _igl, float _igh, unsigned _w, unsigned _h, float _grl, float _grh, float _gm, float _fm, float _frl, float _frh) noexcept
{
	std::random_device rd;
	std::mt19937 mt(rd());

	mnWidth = _w;
	mnHeight = _h;

	// usage layers
	mSpaceLayer.resize(mnHeight);
	for (auto& sub : mSpaceLayer)
		sub.resize(mnWidth, -1);

	mGrassLayer.resize(mnHeight);
	for (auto& sub : mGrassLayer)
		sub.resize(mnWidth, 0.f);

	mFertilizerLayer.resize(mnHeight);
	for (auto& sub : mFertilizerLayer)
		sub.resize(mnWidth, 0.f);

	// growth rates
	std::uniform_real_distribution<float> distR1(_grl, _grh);
	std::uniform_real_distribution<float> distR2(_frl, _frh);
	mGrassLayerRate.resize(mnHeight);
	for (auto& sub : mGrassLayerRate)
		sub.resize(mnWidth, 0.f);
	for (auto& r : mGrassLayerRate)
		for (auto& c : r)
			c = distR1(mt);

	mFertilizerRate.resize(mnHeight);
	for (auto& sub : mFertilizerRate)
		sub.resize(mnWidth, 0.f);
	for (auto& r : mFertilizerRate)
		for (auto& c : r)
			c = distR2(mt);

	// low and high limits
	mGrassThresh.resize(mnHeight);
	for (auto& sub : mGrassThresh)
		sub.resize(mnWidth, std::make_pair(0.f, _gm));

	mFertilizerThresh.resize(mnHeight);
	for (auto& sub : mFertilizerThresh)
		sub.resize(mnWidth, std::make_pair(0.f, _fm));

	// normalize gradient towards centre of grid
	std::uniform_real_distribution<float> dist{};
	unsigned centreRow = mnHeight / 2;
	unsigned centreCol = mnWidth / 2;
	float maxD = sqrtf(static_cast<float>(centreRow*centreRow + centreCol * centreCol));
	for (unsigned i = 0; i < mnHeight; ++i)
	{
		for (unsigned j = 0; j < mnWidth; ++j)
		{
			float d = sqrtf(static_cast<float>((centreRow - i)*(centreRow - i) + (centreCol - j)*(centreCol - j)));
			mFertilizerThresh[i][j].first = 1.f - d / maxD;
			mFertilizerLayer[i][j] = mFertilizerThresh[i][j].first * mFertilizerThresh[i][j].second;
		}
	}

	// node style for path finding
	mNodeLayer.resize(mnHeight);
	for (int i = 0; i < static_cast<int>(mnHeight); ++i)
	{
		mNodeLayer[i].resize(mnWidth, Node{});
		for (int j = 0; j < static_cast<int>(mnWidth); ++j)
			mNodeLayer[i][j] = Node{ GridPos{ j, i}, 0.f, 0.f };
	}

	std::uniform_real_distribution<float> distF(_igl, _igh);
	std::uniform_int_distribution<unsigned> distI(0, mnWidth * mnHeight - 1);

	unsigned times = static_cast<unsigned>(_iga * static_cast<float>(mnWidth * mnHeight));
	for (unsigned i = 0; i < times; ++i)
	{
		unsigned idx = distI(mt);
		unsigned row = idx / mnWidth;
		unsigned col = idx - (row * mnWidth);
		if (mGrassLayer[row][col] > 0)
			--i;
		else
			mGrassLayer[row][col] = distF(mt) * (mGrassThresh[row][col].second - mGrassThresh[row][col].first);
	}
}

const CS380::Map<int>& CS380::Terrain::GetSpaceLayer(void) const noexcept
{
	return mSpaceLayer;
}

CS380::Map<int>& CS380::Terrain::GetSpaceLayer(void) noexcept
{
	return mSpaceLayer;
}

const CS380::Map<float>& CS380::Terrain::GetGrassLayer(void) const noexcept
{
	return mGrassLayer;
}

CS380::Map<float>& CS380::Terrain::GetGrassLayer(void) noexcept
{
	return mGrassLayer;
}

const CS380::Map<float>& CS380::Terrain::GetFertilizerLayer(void) const noexcept
{
	return mFertilizerLayer;
}

CS380::Map<float>& CS380::Terrain::GetFertilizerLayer(void) noexcept
{
	return mFertilizerLayer;
}

const CS380::Map<float>& CS380::Terrain::GetGrassLayerRate(void) const noexcept
{
	return mGrassLayerRate;
}

CS380::Map<float>& CS380::Terrain::GetGrassLayerRate(void) noexcept
{
	return mGrassLayerRate;
}

const CS380::Map<std::pair<float, float>>& CS380::Terrain::GetGrassLayerThresh(void) const noexcept
{
	return mGrassThresh;
}

CS380::Map<std::pair<float, float>>& CS380::Terrain::GetGrassLayerThresh(void) noexcept
{
	return mGrassThresh;
}

const CS380::Map<std::pair<float, float>>& CS380::Terrain::GetFertilizerLayerThresh(void) const noexcept
{
	return mFertilizerThresh;
}

CS380::Map<std::pair<float, float>>& CS380::Terrain::GetFertilizerLayerThresh(void) noexcept
{
	return mFertilizerThresh;
}

void CS380::Terrain::Update(float _dt) noexcept
{
	for (unsigned short y = 0; y < mnHeight; ++y)
	{
		for (unsigned short x = 0; x < mnWidth; ++x)
		{
			mFertilizerLayer[y][x] = Clamp(mFertilizerThresh[y][x].first, mFertilizerThresh[y][x].second, mFertilizerLayer[y][x] + (mFertilizerRate[y][x] * _dt * mFertilizerThresh[y][x].second));

			float rate = mGrassLayerRate[y][x] * _dt * mGrassThresh[y][x].second;
			float consumableValue = Min(mFertilizerLayer[y][x], rate);
			// birthed out by neighbours
			if (mGrassLayer[y][x] >= mGrassThresh[y][x].second)
			{
				auto p = GetLowestGrass({ x,y });
				if (p.x < 0 || p.y < 0)
					continue;
				consumableValue /= 8.f;
				mGrassLayer[p.y][p.x] = Clamp(0.f, mGrassThresh[p.y][p.x].second, mGrassLayer[p.y][p.x] + consumableValue);
			}
			// normal rates
			else
			{
				mGrassLayer[y][x] += consumableValue;
			}
			mFertilizerLayer[y][x] = Clamp(0.f, mFertilizerThresh[y][x].second, mFertilizerLayer[y][x] - consumableValue);
			mFertilizerThresh[y][x].first = mFertilizerLayer[y][x] / mFertilizerThresh[y][x].second;

		}
	}
}

float CS380::Terrain::ConsumeGrass(unsigned _x, unsigned _y, float _val) noexcept
{
	if (_x > mnWidth || _y > mnHeight)
		return 0;

	float v = Min(_val * (mGrassThresh[_y][_x].second - mGrassThresh[_y][_x].first), mGrassLayer[_y][_x]);
	float result = Clamp(mGrassThresh[_y][_x].first, mGrassThresh[_y][_x].second, mGrassLayer[_y][_x] - v);
	v = mGrassLayer[_y][_x] - result;
	mGrassLayer[_y][_x] = result;
	return v;
}

unsigned int CS380::Terrain::GetGrassColor(unsigned _x, unsigned _y) const noexcept
{
	float r = mGrassLayer[_y][_x] / (mGrassThresh[_y][_x].second - mGrassThresh[_y][_x].first);
	return ImGui::GetColorU32(Lerp(ImVec4{ 0.f, 0.3f, 0.f, 0.5f }, 
								   ImVec4{ 0.f, 0.9f, 0.f, 0.8f }, r));
}

std::vector<CS380::GridPos> CS380::Terrain::GetShortestPath(const GridPos& _src, const GridPos& _dest) noexcept
{
	for (auto& row : mNodeLayer)
	{
		for (auto& col : row)
		{
			col.fcost = col.hcost = col.tcost = std::numeric_limits<float>::infinity();
			col.mpPrev = nullptr;
		}
	}
	
	std::vector<CS380::GridPos> result;
	std::priority_queue<Node*, std::vector<Node*>, Comp> q;
	for (auto & n : GetNeighbours(_src, _dest, 0.f, nullptr))
		q.push(n);

	while (!q.empty())
	{
		Node * cur = q.top();
		q.pop();

		if (q.size() > 1000)
			__debugbreak();
		if (cur->pos == _dest)
		{
			Node * p = cur->mpPrev;
			result.insert(result.begin(), cur->pos);
			while (p)
			{
				result.insert(result.begin(), p->pos);
				p = p->mpPrev;
			}
			break;
		}

		for (auto& n : GetNeighbours(cur->pos, _dest, cur->tcost, cur))
			q.push(n);
	}
	return result;
}

CS380::GridPos CS380::Terrain::GetBestGrassPos(const GridPos& _src, float _limit, float _minAlpha) noexcept
{
	for (auto& row : mNodeLayer)
	{
		for (auto& col : row)
		{
			col.fcost = col.hcost = col.tcost = std::numeric_limits<float>::infinity();
			col.mpPrev = nullptr;
		}
	}

	unsigned& w = mnWidth;
	unsigned& h = mnHeight;
	auto getNeigh = [w, h](const GridPos& _src, Map<Node>& _mapLay, float _curT)
	{
		std::vector<CS380::Node*> result;
		for (int j = -1; j < 2; ++j)
		{
			for (int i = -1; i < 2; ++i)
			{
				if (static_cast<unsigned>(_src.x + i) >= w || static_cast<unsigned>(_src.y + j) >= h)
					continue;

				if (_mapLay[_src.y + j][_src.x + i].fcost <= ((!i || !j) ? _curT + 1 : _curT + SQRT_2))
					continue;

				_mapLay[_src.y + j][_src.x + i].hcost = 0.f;
				_mapLay[_src.y + j][_src.x + i].fcost =
					_mapLay[_src.y + j][_src.x + i].tcost =
					(!i || !j) ? _curT + 1 : _curT + SQRT_2;
				result.push_back(&_mapLay[_src.y + j][_src.x + i]);
			}
		}
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(result.begin(), result.end(), g);
		return result;
	};

	std::vector<CS380::GridPos> result;
	std::priority_queue<Node*, std::vector<Node*>, Comp> q;
	for (auto & n : getNeigh(_src, mNodeLayer, 0.f))
		q.push(n);

	while (!q.empty())
	{
		Node * cur = q.top();
		q.pop();

		// if regrowth more than half, then consider it a grass patch
		if (mGrassLayer[cur->pos.y][cur->pos.x] / (mGrassThresh[cur->pos.y][cur->pos.x].second - mGrassThresh[cur->pos.y][cur->pos.x].first) > _minAlpha)
			result.push_back(cur->pos);

		if (static_cast<float>(sqrt((cur->pos.x - _src.x)*(cur->pos.x - _src.x) + (cur->pos.y - _src.y)*(cur->pos.y - _src.y))) < _limit)
			for (auto& n : getNeigh(cur->pos, mNodeLayer, cur->fcost))
				q.push(n);
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(result.begin(), result.end(), g);
	if (result.size())
	{
		unsigned highestId = 0;
		float highestV = mGrassLayer[result[0].y][result[0].x] / (mGrassThresh[result[0].y][result[0].x].second - mGrassThresh[result[0].y][result[0].x].first);
		for (unsigned i = 1; i < result.size(); ++i)
		{
			float v = mGrassLayer[result[i].y][result[i].x] / (mGrassThresh[result[i].y][result[i].x].second - mGrassThresh[result[i].y][result[i].x].first);
			if (v > highestV)
				highestId = i;
		}
		return result[highestId];
	}
	return GridPos{ -1,-1 };
}

std::vector<CS380::Node*> CS380::Terrain::GetNeighbours(const GridPos& _src, const GridPos& _dest, float _curT, Node * _prev) noexcept
{
	std::vector<CS380::Node*> result;
	result.reserve(8);
	for (int j = -1; j < 2; ++j)
	{
		for (int i = -1; i < 2; ++i)
		{
			if (!j && !i)
				continue;
			if (static_cast<unsigned>(_src.x + i) >= mnWidth || 
				static_cast<unsigned>(_src.y + j) >= mnHeight)
				continue;

			float x = static_cast<float>(abs(_dest.x - (_src.x + i)));
			float y = static_cast<float>(abs(_dest.y - (_src.y + j)));

			float h = GetOctileCost(x, y);
			if (mNodeLayer[_src.y + j][_src.x + i].fcost <= h + ((!i || !j) ? _curT + 1 : _curT + SQRT_2))
				continue;

			if (_prev && _prev->mpPrev == &mNodeLayer[_src.y + j][_src.x + i])
				__debugbreak(); //cyclic

			mNodeLayer[_src.y + j][_src.x + i].hcost = h;
			mNodeLayer[_src.y + j][_src.x + i].tcost = (!i || !j) ? _curT + 1 : _curT + SQRT_2;
			mNodeLayer[_src.y + j][_src.x + i].fcost = mNodeLayer[_src.y + j][_src.x + i].hcost + mNodeLayer[_src.y + j][_src.x + i].tcost;
			mNodeLayer[_src.y + j][_src.x + i].mpPrev = _prev;

			result.push_back(&mNodeLayer[_src.y + j][_src.x + i]);
		}
	}
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(result.begin(), result.end(), g);
	return result;
}

CS380::GridPos CS380::Terrain::GetEmptyNeighbour(const GridPos& _src) noexcept
{
	for (int j = -1; j < 2; ++j)
	{
		for (int i = -1; i < 2; ++i)
		{
			if (!j && !i)
				continue;

			if (static_cast<unsigned>(_src.x + i) >= mnWidth ||
				static_cast<unsigned>(_src.y + j) >= mnHeight)
				continue;

			if (mSpaceLayer[_src.y + j][_src.x + i] == -1)
				return GridPos{  _src.x + i, _src.y + j };
		}
	}
	return GridPos{ -1, -1 };
}

CS380::GridPos CS380::Terrain::GetLowestGrass(const GridPos& _src) noexcept
{
	// Random in 8 direction
	std::vector<std::pair<int, int>> neighbourCells = { {-1, 1}, {0, 1}, {1, 1}, 
														{-1, 0},		 {1, 0},
														{-1, -1},{0, -1},{1,-1}
													};
	std::random_device rd; // obtain a random number from hardware
	std::mt19937 eng(rd()); // seed the generator

	CS380::GridPos lowest{ -1, -1 };
	float lowestV = std::numeric_limits<float>::max();
	while (!neighbourCells.empty())
	{	
		std::uniform_int_distribution<> distr(0,static_cast<int>(neighbourCells.size())-1); // define the range

		const int randIdx = distr(eng);

		CS380::GridPos newPos{ 0, 0 };
		newPos = GridPos{ static_cast<int>(_src.x) + neighbourCells[randIdx].first, static_cast<int>(_src.y) + neighbourCells[randIdx].second };
		
		// out of range
		if (static_cast<unsigned>(newPos.x) >= mnWidth ||
			static_cast<unsigned>(newPos.y) >= mnHeight)
		{
			neighbourCells.erase(neighbourCells.begin() + randIdx);
			continue;
		}

		if (mGrassLayer[newPos.y][newPos.x] < lowestV)
		{
			lowestV = mGrassLayer[newPos.y][newPos.x];
			lowest.x = newPos.x;
			lowest.y = newPos.y;
		}
		neighbourCells.erase(neighbourCells.begin() + randIdx);
	}

	return lowest;
}




