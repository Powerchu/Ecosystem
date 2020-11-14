#include "Creatures/Rabbit.h"
#include "EcoSystem/EcoSystem.h"
#include "imgui.h"

CS380::Rabbit::Rabbit(const Traits& _t, unsigned _id) noexcept
	: Creature{ "Rabbit", FLAG_INVALID, _t, _id },
	searching{ false }
{
	// color
	float totalT = _t.mfSense + _t.mfSize + _t.mfSpeed;
	float r = _t.mfSense / totalT;
	float g = _t.mfSize / totalT;
	float b = _t.mfSpeed / totalT;
	Creature::SetColor(0.5f + r, 0.5f + g, b, 1.f);

	// base values
	Creature::SetFatigueBase(std::make_pair(500.f, 1000.f));
	Creature::SetEnergyBase(std::make_pair(500.f, 1000.f));
}

CS380::Rabbit::~Rabbit(void) noexcept
{

}

void CS380::Rabbit::UpdateAwakeBehaviour(float)
{
	//if (GetFlags() & FLAG_TIRED)
	//	mBitFlags |= FLAG_ASLEEP;
	unsigned x = 0;
	unsigned y = 0;
	GetGridPosition(x, y);
	int xDirection[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int yDirection[8] = { 1, -1, 0, -1, 1, 0, 1, -1 };

	if (!searching)
	{
		searching = true;
		auto pos = EcoSystem::GetInst().GetBestGrassPos({ static_cast<int>(x), static_cast<int>(y) }, GetSense(), 0.3f);
		if (pos.x != -1 && GetEnergy().first < (GetEnergy().second * GetEvoData().mfReplicationThresh))
		{
			auto path = EcoSystem::GetInst().GetShortestPath({ static_cast<int>(x), static_cast<int>(y) }, pos);
			if (!path.empty())
				SetMovement(path);
		}
		else
		{
			int randX = 0;
			int randY = 0;
			CS380::GridPos newPos{ 0, 0 };
			bool valid = false;

			while (!valid)
			{
				int prevX = randX;
				int prevY = randY;
				randX = rand() % 8;
				randY = rand() % 8;
				if (prevX == randX || prevY == randY)
				{
					continue;
				}
				newPos = GridPos{ static_cast<int>(x) + xDirection[randX], static_cast<int>(y) + yDirection[randY] };
				if (static_cast<unsigned>(newPos.x) >= static_cast<unsigned>(EcoSystem::GetInst().GetWidth()) ||
					static_cast<unsigned>(newPos.y) >= static_cast<unsigned>(EcoSystem::GetInst().GetHeight()))
				{
					continue;
				}
				valid = true;
			}

			auto path = EcoSystem::GetInst().GetShortestPath({ static_cast<int>(x), static_cast<int>(y) }, newPos);
			if (!path.empty())
				SetMovement(path);
		}
	}
	else
	{
		if (!HasPendingMovement())
		{
			searching = false;
		}
	
		if (EcoSystem::GetInst().GetGrassValA(GetGridPosition().x, GetGridPosition().y) > 0.1f)
		{
			Eat();
		}
		
	}
	
}

void CS380::Rabbit::UpdateAsleepBehaviour(float)
{
}








