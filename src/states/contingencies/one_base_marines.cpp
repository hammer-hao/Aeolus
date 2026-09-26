#include "one_base_marines.h"

#include "../contingency.h"
#include "../consolidate.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"

#include "../../behaviors/macro_behaviors/mining.h"
#include "../../behaviors/macro_behaviors/scout.h"
#include "../../behaviors/macro_behaviors/tech_up.h"
#include "../../behaviors/macro_behaviors/spawn_controller.h"
#include "../../behaviors/macro_behaviors/production_controller.h"
#include "../../behaviors/macro_behaviors/auto_supply.h"
#include "../../behaviors/macro_behaviors/chrono_controller.h"
#include "../../behaviors/macro_behaviors/build_workers.h"
#include "../../behaviors/macro_behaviors/build_structure.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"

namespace Aeolus
{
	std::string_view OneBaseMarines::getName() const
	{
		return "ONE_BASE_MARINES";
	}

	void OneBaseMarines::micro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		int baseToDefend = 1; // defend the natural base
		::sc2::Point2D target = mediator.GetDefenseTarget(aeolusbot, baseToDefend);
		doGeneralMicro(aeolusbot, forces, target);
	}

	void OneBaseMarines::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		// build a shield battery at natural
		auto ownStructure = mediator.GetAllOwnStructures(aeolusbot);
		std::map<::sc2::UNIT_TYPEID, float> army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0} };

		if (!std::any_of(ownStructure.begin(), ownStructure.end(), [](const ::sc2::Unit* structure) {
			return structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY;
			}) && mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY) == 0)
		{
			aeolusbot.RegisterBehavior(std::make_unique<BuildStructure>(
				::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY,
				1,
				mediator.GetDefenseTarget(aeolusbot, 1)
			));
		}

		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<Scout>());
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(army_comp));
		aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(army_comp));

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());
		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(36));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

		if (aeolusbot.Observation()->GetGameLoop() % 22 == 1)
		{
			const int numBarracks = 4;
			auto ownAttacking = ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
			if (ownAttacking.size() > numBarracks)
			{
				aeolusbot.ChangeState(MakeState<ConsolidateState>());
			}
		}
	}
}