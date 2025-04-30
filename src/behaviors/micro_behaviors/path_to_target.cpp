#include "path_to_target.h"
#include "micro_maneuver.h"
#include "move.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../utils/Astar.hpp"
#include "../../pathing/grid.h"

namespace Aeolus
{
	bool PathToTarget::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (::sc2::DistanceSquared2D(unit->pos, m_target) <= 0.0) return false;

		// std::cout << "Path to target: retrieving grid... " << std::endl;
		GridType influenceGridType = (unit->is_flying) ?
			GridType::AIR :
			GridType::GROUND;

		// std::cout << "Path to target: retrieved grid... " << std::endl;

		::sc2::Point2D move_to = ManagerMediator::getInstance().FindNextPathingPoint(
			aeolusbot, 
			influenceGridType, 
			unit->pos, 
			m_target
		);

		//aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::MOVE_MOVE, m_target);
		//return true;

		Move move = Move(move_to);

		return move.execute(aeolusbot, unit);
	}
}
