#include "keep_unit_safe.h"

#include "micro_maneuver.h"
#include "path_to_target.h"
#include "move.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_typeenums.h>
#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"

namespace Aeolus
{
	bool KeepUnitSafe::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		auto& manager = ManagerMediator::getInstance();

		if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPPRISM)
		{
			if (unit->cargo_space_taken == 0 && manager.IsAirPositionSafe(aeolusbot, unit->pos)) return false;
			else if (manager.IsGroundPositionSafe(aeolusbot, unit->pos) && manager.IsAirPositionSafe(aeolusbot, unit->pos)) return false;
		}
		else if (!unit->is_flying && manager.IsGroundPositionSafe(aeolusbot, unit->pos)) return false;
		else if (unit->is_flying && manager.IsAirPositionSafe(aeolusbot, unit->pos)) return false;

		::sc2::Point2D safe_spot = { 0.0, 0.0 };

		if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPPRISM)
		{
			/*safe_spot = m_target.has_value() ?
				manager.FindClosestSafeSpotTowards(aeolusbot, unit->pos, m_target.value(), 7.0, GridType::BOTH) :
				manager.FindClosestPrismSafeSpot(aeolusbot, unit->pos, 7.0);*/

			safe_spot = manager.FindClosestPrismSafeSpot(aeolusbot, unit->pos, 7.0);
		}
		else
		{
			safe_spot = (!unit->is_flying) ?
				manager.FindClosestGroundSafeSpot(aeolusbot, unit->pos, 7.0) :
				manager.FindClosestAirSafeSpot(aeolusbot, unit->pos, 7.0);
			/*if (m_target.has_value())
			{
				GridType gridType = unit->is_flying ? GridType::AIR : GridType::GROUND;
				safe_spot = manager.FindClosestSafeSpotTowards(aeolusbot, unit->pos, m_target.value(), 7.0, gridType);
			}
			else
			{
				safe_spot = (!unit->is_flying) ?
					manager.FindClosestGroundSafeSpot(aeolusbot, unit->pos, 7.0) :
					manager.FindClosestAirSafeSpot(aeolusbot, unit->pos, 7.0);
			}*/
		}

		if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_STALKER &&
			(unit->shield / unit->shield_max) <= 0.1f)
		{
			const auto& availableAbilities = aeolusbot.Query()->GetAbilitiesForUnit(unit);
			for (const auto& ability : availableAbilities.abilities)
			{
				if (ability.ability_id.ToType() == ::sc2::ABILITY_ID::EFFECT_BLINK)
				{
					// blink available, use it!
					aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::EFFECT_BLINK, safe_spot);
					return true;
				}
			}
		}

		std::cout << "[KeepUnitSafe] unit: " << ::sc2::UnitTypeToName(unit->unit_type) << ", position: (" <<
			unit->pos.x << ", " << unit->pos.y << "), target: (" << safe_spot.x << ", " << safe_spot.y << ')' << std::endl;

		// blink not available, just path unit to target
		auto path = PathToTarget(safe_spot);
		return path.execute(aeolusbot, unit);
	}
}