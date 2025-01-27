#include "keep_unit_safe.h"

#include "micro_maneuver.h"
#include "path_to_target.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include "../../managers/manager_mediator.h"

namespace Aeolus
{
	bool KeepUnitSafe::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		auto& manager = ManagerMediator::getInstance();
		
		if (manager.IsGroundPositionSafe(aeolusbot, unit->pos)) return false;

		::sc2::Point2D safe_spot = manager.FindClosestGroundSafeSpot(aeolusbot, unit->pos, 7.0);
		auto path = PathToTarget(safe_spot);
		return path.execute(aeolusbot, unit);
	}
}