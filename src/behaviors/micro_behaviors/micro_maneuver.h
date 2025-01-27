#pragma once

#include "../behavior.h"
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;
	/* Interface all macro maneuver should adhere to.
	All micro maneuver must have the execute() member function,
	which takes in constant reference of the bot and mediator
	for infromation retrieval only. */
	class MicroManeuver : public Behavior
	{
	public:
		~MicroManeuver() override = default;

		bool execute(AeolusBot& aeolusbot) final override { return false; }

		virtual bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) = 0;
	};
}