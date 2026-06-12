#include "move_toward_target_safely.h"

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include "../../utils/unit_utils.h"
#include "../../pathing/grid.h"
#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"

#include "move.h"

namespace Aeolus
{
	bool MoveTowardTargetSafely::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		auto& mediator = ManagerMediator::getInstance();

		double attackRange = 0.0f;
		if (m_target->is_flying)
		{
			attackRange = mediator.AirRange(aeolusbot, unit);
		}
		else
		{
			attackRange = mediator.GroundRange(aeolusbot, unit);
		}
		
		::sc2::Point2D safeSpot = unit->is_flying ?
			mediator.FindFurthestAirSafeSpotTowards(aeolusbot, m_target->pos, unit->pos, attackRange) :
			mediator.FindFurthestGroundSafeSpotTowards(aeolusbot, m_target->pos, unit->pos, attackRange);
		
		bool found = unit->is_flying ?
			mediator.IsAirPositionSafe(aeolusbot, safeSpot) :
			mediator.IsGroundPositionSafe(aeolusbot, safeSpot);

		if (!found) return false;

		Move move(safeSpot);
		return move.execute(aeolusbot, unit);
	}
}