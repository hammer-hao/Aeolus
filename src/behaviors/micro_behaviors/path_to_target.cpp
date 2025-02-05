#include "path_to_target.h"
#include "micro_maneuver.h"
#include "move.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../utils/Astar.hpp"

namespace Aeolus
{
	bool PathToTarget::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (::sc2::DistanceSquared2D(unit->pos, m_target) <= 0.0) return false;

		::sc2::Point2D move_to = ManagerMediator::getInstance().FindNextPathingPoint(aeolusbot, unit->pos, m_target);

		//aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::MOVE_MOVE, m_target);
		//return true;

		Move move = Move(move_to);

		return move.execute(aeolusbot, unit);
	}
}
