#include "a_move.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include "../../Aeolus.h"

namespace Aeolus
{
	bool AMove::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (unit->orders.size() == 1)
		{
			if (unit->orders.front().ability_id == ::sc2::ABILITY_ID::ATTACK
				&& unit->orders.front().target_pos == m_target)
				// already performing the exact command
				return true;
		}
		aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::ATTACK, m_target);
		return true;
	}
}