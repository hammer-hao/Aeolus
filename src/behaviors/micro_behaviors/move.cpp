#include "move.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"

namespace Aeolus
{
	bool Move::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (unit->orders.size() == 1)
		{
			if (unit->orders.front().ability_id == ::sc2::ABILITY_ID::MOVE_MOVE
				&& unit->orders.front().target_pos == m_target)
				// already performing the exact command
				std::cout << "duplicate command detected!" << std::endl;
				return true;
		}
		aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::MOVE_MOVE, m_target);
		return true;
	}
}