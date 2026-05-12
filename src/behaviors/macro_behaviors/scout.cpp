#include "scout.h"
#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../constants.h"

#include "../micro_behaviors/keep_unit_safe.h"
#include "../micro_behaviors/path_to_target.h"
#include "../micro_behaviors/move.h"
#include "../micro_behaviors/micro_maneuver.h"
#include "../micro_behaviors/micro_behavior.h"

#include <sc2lib/sc2_lib.h>
#include <sc2lib/sc2_search.h>


namespace Aeolus
{
	bool Scout::execute(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();

		auto scoutingUnits = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::SCOUTING);

		for (const auto* scoutingUnit : scoutingUnits)
		{
			auto target = mediator.checkScoutingWayPoint(aeolusbot, scoutingUnit);

			if (target.x == 0.0f || target.y == 0.0f) continue;

			auto scout_behavior = std::make_unique<MicroBehavior>(scoutingUnit);
			scout_behavior->AddBehavior(std::make_unique<Move>(target));
			scout_behavior->execute(aeolusbot);
		}

		return true;
	}
}