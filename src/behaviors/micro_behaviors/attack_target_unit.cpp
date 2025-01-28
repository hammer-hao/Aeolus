#include "attack_target_unit.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"

namespace Aeolus
{
	bool AttackTargetUnit::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (unit->orders.size() == 1)
		{
			if (unit->orders.front().ability_id == ::sc2::ABILITY_ID::ATTACK
				&& aeolusbot.Observation()->GetUnit(unit->orders.front().target_unit_tag) == m_target)
				// already performing the exact command
				return true;
		}
		aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::ATTACK, m_target);
		return true;
	}
}