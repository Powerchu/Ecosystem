#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <vector>

namespace Ecosystem
{
	template<typename T>
	using Map = std::vector<std::vector<T>>;

	struct GridPos
	{
		GridPos(int _x, int _y) noexcept;
		bool operator==(const GridPos& _rhs) const; 
		int x;
		int y;
	};

	struct Node
	{
		Node(const GridPos& _p, float _t, float _h) noexcept;
		Node(void) noexcept;
		bool operator<(const Node& _rhs) const noexcept;
		GridPos pos;
		float tcost;
		float hcost;
		float fcost;
		Node* mpPrev;
	};

	class Terrain
	{
	public:
		Terrain(unsigned _x = 0, unsigned _y = 0);
		
		void Init(float _iga, float _igl, float _igh, unsigned _w, unsigned _h, float _grl, float _grh, float _gm, float _fm, float _frl, float _frh) noexcept;

		const Map<int>& GetSpaceLayer(void) const noexcept;
		Map<int>& GetSpaceLayer(void) noexcept;

		const Map<float>& GetGrassLayer(void) const noexcept;
		Map<float>& GetGrassLayer(void) noexcept;

		const Map<float>& GetFertilizerLayer(void) const noexcept;
		Map<float>& GetFertilizerLayer(void) noexcept;

		const Map<float>& GetGrassLayerRate(void) const noexcept;
		Map<float>& GetGrassLayerRate(void) noexcept;

		const Map<std::pair<float, float>>& GetGrassLayerThresh(void) const noexcept;
		Map<std::pair<float, float>>& GetGrassLayerThresh(void) noexcept;

		// first is normal, second is max
		const Map<std::pair<float, float>>& GetFertilizerLayerThresh(void) const noexcept;
		Map<std::pair<float, float>>& GetFertilizerLayerThresh(void) noexcept;

		void Update(float) noexcept;

		float ConsumeGrass(unsigned _x, unsigned _y, float _val) noexcept;

		unsigned int GetGrassColor(unsigned _x, unsigned _y) const noexcept;

		// A*
		std::vector<GridPos> GetShortestPath(const GridPos& _source, const GridPos& _dest) noexcept;
		GridPos GetBestGrassPos(const GridPos& _source, float _mnLimit, float _minAlpha) noexcept;
		GridPos GetEmptyNeighbour(const GridPos& _src) noexcept;

	private:

		Map<int> mSpaceLayer;
		Map<float> mGrassLayer;
		Map<float> mFertilizerLayer;
		Map<Node> mNodeLayer;

		Map<float> mGrassLayerRate;
		Map<float> mFertilizerRate;

		// std pair low,high
		Map<std::pair<float,float>> mGrassThresh;
		// first is normal, second is max
		Map<std::pair<float,float>> mFertilizerThresh;

		unsigned mnWidth;
		unsigned mnHeight;

		std::vector<Node*> GetNeighbours(const GridPos& _src, const GridPos& _dest, float _curT, Node * _prev) noexcept;
		template<typename T>
		void Normalize(Map<T>& _m);
		GridPos GetLowestGrass(const GridPos& _src) noexcept;
	};

	template<typename T>
	inline void Terrain::Normalize(Map<T>& _m)
	{
		T largest = _m[0][0];
		for (unsigned i = 0; i < _m.size(); ++i)
			for (unsigned j = 0; j < _m[i].size(); ++j)
				if (_m[i][j] > largest)
					largest = _m[i][j];

		for (unsigned i = 0; i < _m.size(); ++i)
			for (unsigned j = 0; j < _m[i].size(); ++j)
				_m[i][j] /= largest;
	}

}



#endif



