#include "keep_unit_safe.h"

#include "micro_maneuver.h"
#include "path_to_target.h"
#include "move.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_typeenums.h>
#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"

#include "../../utils/position_utils.h"

namespace Aeolus
{
	bool KeepUnitSafe::execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		auto& manager = ManagerMediator::getInstance();

		bool locked_on = std::any_of(unit->buffs.begin(), unit->buffs.end(), [](::sc2::BUFF_ID buff) {
			return buff == ::sc2::BUFF_ID::LOCKON;
			});

		if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPPRISM)
		{
			if (!locked_on && unit->cargo_space_taken == 0 && manager.IsAirPositionSafe(aeolusbot, unit->pos)) return false;
			else if (!locked_on && manager.IsGroundPositionSafe(aeolusbot, unit->pos) && manager.IsAirPositionSafe(aeolusbot, unit->pos)) return false;
		}
		else if (!locked_on && !unit->is_flying && manager.IsGroundPositionSafe(aeolusbot, unit->pos)) return false;
		else if (!locked_on && unit->is_flying && manager.IsAirPositionSafe(aeolusbot, unit->pos)) return false;

		::sc2::Point2D safe_spot = { 0.0, 0.0 };

		if (locked_on)
		{
			::sc2::Point2D starting_point = unit->pos;
			::sc2::Units all_close = manager.GetUnitsInRange(aeolusbot, { starting_point }, 15.0f);
			::sc2::Units cyclones;
			std::copy_if(all_close.begin(), all_close.end(), std::back_inserter(cyclones),
				[](const ::sc2::Unit* unit_) {return unit_->unit_type == ::sc2::UNIT_TYPEID::TERRAN_CYCLONE;  });
			if (cyclones.empty())
			{
				locked_on = false;
			}
			else
			{
				std::sort(cyclones.begin(), cyclones.end(), [&](const ::sc2::Unit* unitA, const ::sc2::Unit* unitB) {
					return ::sc2::DistanceSquared2D(unitA->pos, unit->pos) < ::sc2::DistanceSquared2D(unitB->pos, unit->pos);
					});
				safe_spot = utils::GetPositionTowards(cyclones.front()->pos, unit->pos, 15.0f, false);
			}
		}
		if (!locked_on)
		{
			if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPPRISM ||
				unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_COLOSSUS)
			{
				safe_spot = manager.FindClosestPrismSafeSpot(aeolusbot, unit->pos, 7.0);
			}
			else
			{
				safe_spot = (!unit->is_flying) ?
					manager.FindClosestGroundSafeSpot(aeolusbot, unit->pos, 7.0) :
					manager.FindClosestAirSafeSpot(aeolusbot, unit->pos, 7.0);
			}
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

		/*std::cout << "[KeepUnitSafe] unit: " << ::sc2::UnitTypeToName(unit->unit_type) << ", position: (" <<
			unit->pos.x << ", " << unit->pos.y << "), target: (" << safe_spot.x << ", " << safe_spot.y << ')' << std::endl;*/

		// blink not available, just path unit to target
		auto path = PathToTarget(safe_spot);
		return path.execute(aeolusbot, unit);
	}
}