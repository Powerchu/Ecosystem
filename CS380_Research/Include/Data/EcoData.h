#ifndef _ECO_DATA_H_
#define _ECO_DATA_H_
#include "Creatures/Creature.h"
#include "EcoSystem/EcoSystem.h"

#define CREATURE_COUNT 2
#include "Creatures/Rabbit.h"
#include "Creatures/Fox.h"

#define EVOLUTION_CHART_COUNT 2

#include <tuple>

namespace CS380
{
	using CreatureList = std::tuple<Rabbit, Fox>;
	static const char * Spawnables[CREATURE_COUNT] = { 
		"Rabbit 1", "Fox1"
	};

	extern EvolutionData EvolutionChart[EVOLUTION_CHART_COUNT];

	namespace Data
	{
		void MakeTools(void);

		template<typename T, typename SFNAE = std::enable_if_t<std::is_base_of_v<Creature, T>, T>>
		void SpawnCreature(unsigned short _x, unsigned short _y, const EvolutionData& _evo, const Traits& _trait, int _i)
		{
			EcoSystem& eco = EcoSystem::GetInst();
			Creature *c = static_cast<Creature*>(new T{ _trait, static_cast<unsigned>(_i) });
			c->SetEvolutionData(_evo);
			c->SetGridPosition(_x, _y);
			c->MarkTerritory();
			eco.AddCreature(c);
		}

		struct SpawnVisitor
		{
			SpawnVisitor(unsigned short _x, unsigned short _y, const EvolutionData& _evo, const Traits& _t) noexcept
				: mnGridX{ _x }, mnGridY{ _y }, mEvo{ _evo }, mTrait{ _t }
			{}

			template<typename T>
			SpawnVisitor(T _x, T _y, const EvolutionData& _evo, const Traits& _t) noexcept
				: SpawnVisitor{ static_cast<unsigned short>(_x), static_cast<unsigned short>(_y), _evo, _t }
			{}

			template<typename T>
			void operator()(int _i)
			{
				Data::SpawnCreature<T>(mnGridX, mnGridY, mEvo, mTrait, _i);
			}

			unsigned short mnGridX;
			unsigned short mnGridY;
			EvolutionData mEvo;
			Traits mTrait;
		};

		template<size_t I>
		struct VisitTuple_Impl
		{
			template <typename T, typename F>
			static void Visit(int _i, F _func)
			{
				if (_i == I - 1)
					_func.operator()<std::tuple_element_t<I - 1, T>>(_i);
				else
					VisitTuple_Impl<I - 1>::template Visit<T>(_i, _func);
			}
		};

		template<>
		struct VisitTuple_Impl<0>
		{
			template <typename T, typename F>
			static void Visit(int, F)
			{
			}
		};

		template<typename F>
		void VisitSpawnTuple(F _func, int _i)
		{
			VisitTuple_Impl<std::tuple_size_v<CreatureList>>::Visit<CreatureList>(_i, _func);
		}
	}
}



#endif


