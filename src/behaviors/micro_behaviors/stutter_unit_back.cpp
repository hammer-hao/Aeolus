#include "stutter_unit_back.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../utils/unit_utils.h"
#include "keep_unit_safe.h"

namespace Aeolus
{
	bool StutterUnitBack::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		auto& mediator = ManagerMediator::getInstance();

		if (utils::isAttackReady(aeolusbot, unit, m_target))
		{
			aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::ATTACK, m_target);
			return true;
		}

		KeepUnitSafe keep_unit_safe = KeepUnitSafe();
		
		return keep_unit_safe.execute(aeolusbot, unit);
	}
}