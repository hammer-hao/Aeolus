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

        constexpr float direct_move_distance = 2.0f;

        if (::sc2::DistanceSquared2D(unit->pos, m_target) <
            direct_move_distance * direct_move_distance)
        {
            Move move(m_target);
            return move.execute(aeolusbot, unit);
        }

        const GridType gridType = unit->is_flying
            ? ((unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPPRISM)
                ? GridType::BOTH : GridType::AIR)
            : GridType::GROUND;

        float lookahead_distance = 2.0f;

        if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ORACLE) lookahead_distance = 5.0f;
        else if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_VOIDRAY) lookahead_distance = 3.0f;

        const auto next = ManagerMediator::getInstance().FindNextPathingPoint(
            aeolusbot, gridType, unit->pos, m_target, true, 20, 5.0f, true,
            5, lookahead_distance);
        if (!next)
        {
            Move move(m_target);
            return move.execute(aeolusbot, unit);
        }

        Move move(*next);
        return move.execute(aeolusbot, unit);
	}
}
