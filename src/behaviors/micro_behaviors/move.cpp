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
		// std::cout << "Existing order size: " << unit->orders.size() << std::endl;
		if (unit->orders.size() == 1)
		{
			//std::cout << "Existing order: " << ::sc2::AbilityTypeToName(unit->orders.front().ability_id) << std::endl;
			// std::cout << "Existing order target: " << unit->orders.front().target_pos.x << " " << unit->orders.front().target_pos.y
			//	<< std::endl;
			// std::cout << "New order target: " << m_target.x << " " << m_target.y << std::endl;
 			if (unit->orders.front().ability_id == ::sc2::ABILITY_ID::GENERAL_MOVE
				&& unit->orders.front().target_pos == m_target)
				// already performing the exact command
				// std::cout << "duplicate command detected!" << std::endl;
				return true;
		}
		aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::GENERAL_MOVE, m_target);
		return true;
	}
}