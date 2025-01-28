#include "stutter_unit_back.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../utils/unit_utils.h"
#include "keep_unit_safe.h"
#include "attack_target_unit.h"

namespace Aeolus
{
	bool StutterUnitBack::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		auto& mediator = ManagerMediator::getInstance();

		if (utils::isAttackReady(aeolusbot, unit, m_target))
		{
			AttackTargetUnit attack_target_unit = AttackTargetUnit(m_target);
			return attack_target_unit.execute(aeolusbot, unit);
		}

		KeepUnitSafe keep_unit_safe = KeepUnitSafe();
		return keep_unit_safe.execute(aeolusbot, unit);
	}
}