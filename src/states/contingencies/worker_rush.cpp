#include "worker_rush.h"

#include "../contingency.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"

#include "../../behaviors/macro_behaviors/mining.h"
#include "../../behaviors/macro_behaviors/scout.h"
#include "../../behaviors/macro_behaviors/build_workers.h"
#include "../../behaviors/macro_behaviors/chrono_controller.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"
#include "../../behaviors/macro_behaviors/spawn_controller.h"
#include "../../behaviors/macro_behaviors/production_controller.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"
#include "../../behaviors/macro_behaviors/auto_supply.h"
#include "../../behaviors/macro_behaviors/tech_up.h"

#include "../consolidate.h"

namespace Aeolus
{
	std::string_view WorkerRush::getName() const
	{
		return "WORKER_RUSH";
	}

	void WorkerRush::micro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		int baseToDefend = 0; // defend the main base
		::sc2::Point2D target = mediator.GetDefenseTarget(aeolusbot, baseToDefend);
		doGeneralMicro(aeolusbot, forces, target);
	}

	void WorkerRush::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<Scout>());
		// prioritize unit production
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(m_army_comp));

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(22));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

		// check if we are safe
		if (aeolusbot.Observation()->GetGameLoop() % 22 != 1) return;
		auto allEnemy = mediator.GetAllSeenEnemyUnits(aeolusbot);
		int countEnemyWorkers = std::count_if(allEnemy.begin(), allEnemy.end(), [](const ::sc2::Unit* enemy) {
			return (enemy->unit_type == ::sc2::UNIT_TYPEID::TERRAN_SCV ||
				enemy->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PROBE ||
				enemy->unit_type == ::sc2::UNIT_TYPEID::ZERG_DRONE);
			});
		if (mediator.GetOwnWorkers(aeolusbot).size() > countEnemyWorkers * 2)
		{
			aeolusbot.ChangeState(MakeState<ConsolidateState>());
		}
	}
}