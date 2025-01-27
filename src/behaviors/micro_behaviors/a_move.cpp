#include "a_move.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include "../../Aeolus.h"

namespace Aeolus
{
	bool AMove::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::ATTACK, m_target);
		return true;
	}
}