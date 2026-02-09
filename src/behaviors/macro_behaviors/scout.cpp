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
#include <sc2api/sc2_map_info.h>


namespace Aeolus
{
	bool Scout::execute(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();

		auto scoutingUnits = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::SCOUTING);

		for (const auto* scoutingUnit : scoutingUnits)
		{
			auto* debug = aeolusbot.Debug();
			const auto height_map = ::sc2::HeightMap(aeolusbot.Observation()->GetGameInfo());

#ifndef BUILD_FOR_LADDER
			float unitz = height_map.TerrainHeight({ static_cast<int>(scoutingUnit->pos.x), static_cast<int>(scoutingUnit->pos.y) });
			debug->DebugSphereOut(
				sc2::Point3D(scoutingUnit->pos.x, scoutingUnit->pos.y, unitz),
				1.0,
				::sc2::Colors::Red);
#endif // 

			auto target = mediator.checkScoutingWayPoint(aeolusbot, scoutingUnit);

			if (target.x == 0.0f || target.y == 0.0f) continue;

#ifndef BUILD_FOR_LADDER
			float targetz = height_map.TerrainHeight({ static_cast<int>(target.x), static_cast<int>(target.y) });
			debug->DebugSphereOut(
				sc2::Point3D(target.x, target.y, unitz),
				1.0,
				::sc2::Colors::Red);
			debug->SendDebug();
#endif // 

			auto scout_behavior = std::make_unique<MicroBehavior>(scoutingUnit);
			scout_behavior->AddBehavior(std::make_unique<Move>(target));
			scout_behavior->execute(aeolusbot);
		}

		return true;
	}
}