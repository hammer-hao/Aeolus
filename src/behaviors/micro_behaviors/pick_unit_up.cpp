#include "pick_unit_up.h"
#include "../../Aeolus.h"
#include "path_to_target.h"

namespace Aeolus
{
	bool PickUnitUp::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (m_target == nullptr) return false;

		if (::sc2::DistanceSquared2D(unit->pos, m_target->pos) <= 25.0f)
		{
			aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::SMART, m_target);
		}
		else
		{
			// out of pick up range for warp prism
			// path to it
			PathToTarget pathToTarget = PathToTarget(m_target->pos);
			pathToTarget.execute(aeolusbot, unit);
		}
		return true;
	}
}