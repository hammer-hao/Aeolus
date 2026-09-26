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
        if (!unit)
            return false;

        // Tune this tolerance to your behavior scheduler and desired arrival radius.
        constexpr float arrival_distance = 0.1f;
        if (::sc2::DistanceSquared2D(unit->pos, m_target) <=
            arrival_distance * arrival_distance)
            return false;

        if (::sc2::DistanceSquared2D(unit->pos, m_target) < 10)
        {
            Move move(m_target);
            return move.execute(aeolusbot, unit);
        }

        const GridType gridType = unit->is_flying
            ? ((unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPPRISM)
                ? GridType::BOTH : GridType::AIR)
            : GridType::GROUND;

        const auto next = ManagerMediator::getInstance().FindNextPathingPoint(
            aeolusbot, gridType, unit->pos, m_target);
        if (!next)
        {
            Move move(m_target);
            return move.execute(aeolusbot, unit);
        }

        Move move(*next);
        return move.execute(aeolusbot, unit);
	}
}
