#include "Creatures/Fox.h"
#include "EcoSystem/EcoSystem.h"
#include "Creatures/Rabbit.h"
#include "imgui.h"
#include <cmath>

Ecosystem::Fox::Fox(const Traits& _t, unsigned _id) noexcept
	: Creature{ "Fox", FLAG_INVALID, _t, _id },
	searching{ false }
{
	// color
	float totalT = _t.mfSense + _t.mfSize + _t.mfSpeed;
	float r = _t.mfSense / totalT;
	float g = _t.mfSize / totalT;
	float b = _t.mfSpeed / totalT;
	Creature::SetColor(0.5f + r, g, 0.3f + b, 1.f);

	// base values
	Creature::SetFatigueBase(std::make_pair(800.f, 1000.f));
	Creature::SetEnergyBase(std::make_pair(1600.f, 2000.f));
}

Ecosystem::Fox::~Fox(void) noexcept
{

}

void Ecosystem::Fox::UpdateAwakeBehaviour(float)
{
	//if (GetFlags() & FLAG_TIRED)
	//	mBitFlags |= FLAG_ASLEEP;

	if (GetEnergy().first < 0.1f * GetEnergy().second)
	{
		isHungry = true;
	}
	else
	{
		isHungry = false;
	}

	if (!searching)
	{
		int xDirection[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
		int yDirection[8] = { 1, -1, 0, -1, 1, 0, 1, -1 };
		searching = true;
		unsigned x = 0;
		unsigned y = 0;
		GetGridPosition(x, y);

		if(static_cast<int>(GetSense()) >= 1)
		{
			for (int rad = 1; rad <= static_cast<int>(GetSense()); rad++)
			{
				if (preyFound) break;

				for (int i = 0; i < 8; ++i)
				{
					Ecosystem::GridPos nPrey{ static_cast<int>(x) + xDirection[i] * rad, static_cast<int>(y) + yDirection[i] * rad };
					int index = Ecosystem::EcoSystem::GetInst().GetGridVal(static_cast<unsigned>(nPrey.x), static_cast<unsigned>(nPrey.y));
					if (index != -1)
					{
						Creature* target = Ecosystem::EcoSystem::GetInst().GetCreature(index);
						if (target && dynamic_cast<Rabbit*>(target))
						{
							if (GetSize() / target->GetSize() >= 1.2f)
							{
								auto path = EcoSystem::GetInst().GetShortestPath({ static_cast<int>(x), static_cast<int>(y) }, nPrey);
								if (!path.empty())
								{
									SetMovement(path);
								}
								preyFound = true;
								break;
							}
						}
						if (target != this && dynamic_cast<Fox*>(target))
						{
							if (GetSize() / target->GetSize() < 1.2f) //run away
							{
								
							}

						}
					}
				}
			}
		}
		
		if (!preyFound)
		{
			if (!isHungry)
			{
				//// Wander
				//int randX = 0;
				//int randY = 0;
				//Ecosystem::GridPos newPos{ 0, 0 };
				//bool valid = false;

				//while (!valid)
				//{
				//	int prevX = randX;
				//	int prevY = randY;
				//	randX = rand() % 8;
				//	randY = rand() % 8;
				//	if (prevX == randX || prevY == randY)
				//	{
				//		continue;
				//	}
				//	newPos = GridPos{ static_cast<int>(x) + xDirection[randX], static_cast<int>(y) + yDirection[randY] };
				//	if (static_cast<unsigned>(newPos.x) >= static_cast<unsigned>(EcoSystem::GetInst().GetWidth()) ||
				//		static_cast<unsigned>(newPos.y) >= static_cast<unsigned>(EcoSystem::GetInst().GetHeight()))
				//	{
				//		continue;
				//	}
				//	valid = true;
				//	break;
				//}

				//auto path = EcoSystem::GetInst().GetShortestPath({ static_cast<int>(x), static_cast<int>(y) }, newPos);
				//if (!path.empty())
				//	SetMovement(path);
			}
			else
			{
				auto pos = EcoSystem::GetInst().GetBestGrassPos({ static_cast<int>(x), static_cast<int>(y) }, GetSense(), 0.3f);
				if (pos.x != -1 && GetEnergy().first < (GetEnergy().second * GetEvoData().mfReplicationThresh))
				{
					auto path = EcoSystem::GetInst().GetShortestPath({ static_cast<int>(x), static_cast<int>(y) }, pos);
					if (!path.empty())
						SetMovement(path);
				}
			}
		}
	}
	else
	{
		if (preyFound)
		{
			if (isHungry)
			{
				Eat();
			}
			else
			{
				if (!HasPendingMovement())
				{
					Eat();
				}
			}
		}
		else
		{
			if (EcoSystem::GetInst().GetGrassValA(GetGridPosition().x, GetGridPosition().y) > 0.2f && isHungry)
			{
				Eat();
			}
		}
		

		if (!HasPendingMovement())
		{
			searching = false;

			if(preyFound)
			{
				preyFound = false;
			}
		}
		
	}
}

void Ecosystem::Fox::UpdateAsleepBehaviour(float)
{
	/*std::pair<float, float> e = GetEnergy();
	if (e.first / e.second > 0.9f)
		mBitFlags &= ~FLAG_ASLEEP;

	unsigned x, y;
	GetGridPosition(x, y);
	EcoSystem::GetInst().HighlightGrid(static_cast<unsigned short>(x), static_cast<unsigned short>(y), ImGui::GetColorU32(ImVec4{ 0.f,0.f,0.f,1.f }));*/
}