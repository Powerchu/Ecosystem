#ifndef _RABBIT_H_
#define _RABBIT_H_
#include "Creature.h"

namespace CS380
{
	class Rabbit : public Creature
	{
	public:
		Rabbit(const Traits& _t, unsigned _id) noexcept;
		~Rabbit(void) noexcept;

		void UpdateAwakeBehaviour(float);
		void UpdateAsleepBehaviour(float);


	private:
		bool searching;
		bool predFound;
	};
}


#endif




