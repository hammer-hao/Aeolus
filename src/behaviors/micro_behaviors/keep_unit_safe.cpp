#include "keep_unit_safe.h"

#include "micro_maneuver.h"
#include "path_to_target.h"
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
		
		if (manager.IsGroundPositionSafe(aeolusbot, unit->pos)) return false;

		::sc2::Point2D safe_spot = manager.FindClosestGroundSafeSpot(aeolusbot, unit->pos, 7.0);

		if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_STALKER)
		{
			const auto& availableAbilities = aeolusbot.Query()->GetAbilitiesForUnit(unit);
			for (const auto& ability : availableAbilities.abilities)
			{
				std::cout << ::sc2::AbilityTypeToName(ability.ability_id.ToType()) << std::endl;
				if (ability.ability_id.ToType() == ::sc2::ABILITY_ID::EFFECT_BLINK)
				{
					// blink available, use it!
					aeolusbot.Actions()->UnitCommand(unit, ::sc2::ABILITY_ID::EFFECT_BLINK, safe_spot);
					return true;
				}
			}
		}
		// blink not available, just path unit to target
		auto path = PathToTarget(safe_spot);
		return path.execute(aeolusbot, unit);
	}
}