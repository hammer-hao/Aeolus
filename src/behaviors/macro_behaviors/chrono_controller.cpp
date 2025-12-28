#include "chrono_controller.h"

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include <sc2api/sc2_typeenums.h>

namespace Aeolus
{
	bool ChronoController::execute(AeolusBot& aeolusbot)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();

		for (const auto& nexus : mediator.GetOwnReadyTownHalls(aeolusbot))
		{
			if (nexus->energy < 50) continue;
			if (aeolusbot.Observation()->GetGameLoop() < 120) continue;

			std::vector<::sc2::Point2D> startingPoints = { nexus->pos };
			const auto nearUnits = ManagerMediator::getInstance().GetOwnUnitsInRange(aeolusbot, startingPoints, 12.0f);
			for (const auto unit : nearUnits)
			{
				if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ORACLE)
				{
					aeolusbot.Actions()->UnitCommand(nexus, 4126, unit); // 4126 for energy recharge
					return true;
				}
			}

			for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
			{
				if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_NEXUS
					|| structure->build_progress < 1.0) continue;
				
				if (structure->orders.empty() || !structure->buffs.empty()) continue;

				aeolusbot.Actions()->UnitCommand(nexus, ::sc2::ABILITY_ID::EFFECT_CHRONOBOOSTENERGYCOST, structure);
				return true;
			}

			// chrono ready, but no structure available;
			return false;
		}
		// no chrono ready
		return false;
	}
}