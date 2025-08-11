#ifndef _SPAWN_TOOL_H_
#define _SPAWN_TOOL_H_
#include "Tools.h"


namespace Ecosystem
{
	class SpawnTool : public Tools
	{
	public:
		SpawnTool(bool _opened = true) noexcept;
		~SpawnTool(void) noexcept;

		void Render(void) noexcept;

	private:

		int mnSpawnX;
		int mnSpawnY;
		int mnCurrSelection;
		int mnSpawnCount;
		float mfCurSize;
		float mfCurSpeed;
		float mfCurSense;
	
	};
}

#endif 



