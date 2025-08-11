#ifndef _FOX_H_
#define _FOX_H_
#include "Creature.h"

namespace Ecosystem
{
	class Fox : public Creature
	{
	public:
		Fox(const Traits& _t, unsigned _id) noexcept;
		~Fox(void) noexcept;

		void UpdateAwakeBehaviour(float);
		void UpdateAsleepBehaviour(float);


	private:
		bool searching;
		bool isHungry;
		bool preyFound;
		[[maybe_unused]] bool predFound;
	};
}


#endif




