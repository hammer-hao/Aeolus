#include "contingency.h"

#include "forward_pressure.h"

#include "bot_state.h"
#include "base_state.h"
#include "../buildorder/contingency_plan.h"
#include "../Aeolus.h"

#include "../managers/manager_mediator.h"

#include "../behaviors/macro_behaviors/auto_supply.h"
#include "../behaviors/macro_behaviors/production_controller.h"
#include "../behaviors/macro_behaviors/spawn_controller.h"
#include "../behaviors/macro_behaviors/build_structure.h"
#include "../behaviors/macro_behaviors/tech_up.h"
#include "../behaviors/macro_behaviors/build_geysers.h"
#include "../behaviors/macro_behaviors/expand.h"
#include "../behaviors/macro_behaviors/build_workers.h"

namespace Aeolus
{
	ContingencyState::ContingencyState(const ContingencyPlan& contingencyPlan) : m_plan(contingencyPlan), m_build_defense_queued(false)
	{
	}

	std::string_view ContingencyState::getName() const
	{
		return "CONTINGENCY";
	}

	void ContingencyState::micro(AeolusBot& aeolusbot)
	{
		if (aeolusbot.Observation()->GetGameLoop() < 100) return;
		auto& mediator = ManagerMediator::getInstance();

		// during the build order, we generally want to defend. However, we would still like to move out
		// if we have enought supply
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		if (forces.empty()) return;

		::sc2::Point2D target = (mediator.GetDefenseTarget(aeolusbot, 1));

		if (!forces.empty()) doGeneralMicro(aeolusbot, forces, target);

		// Enable prism pick up during contingency if one has been created (not likely)
		doPrismPickUpMicro(aeolusbot);

		// We are open to doing Oracle harass during the contingency stage
		// doOracleDefensiveMicro(aeolusbot);
	}

	void ContingencyState::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		doBookKeepingMacroTasks(aeolusbot);

		if (!m_build_defense_queued)
		{
			bool stillBuildingForge = false;
			if (mediator.IsStructureAvailable(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_FORGE))
			{
				for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
				{
					if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_FORGE &&
						structure->build_progress > 0.75f)
					{
						for (int i = 0; i < m_plan.cannons_to_add; ++i)
						{
							const ::sc2::UNIT_TYPEID to_build = ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON;
							const int base_location = 1;
							const bool is_wall = true;
							aeolusbot.RegisterBehavior(std::make_unique<BuildStructure>(to_build, base_location, is_wall));
						}
						m_build_defense_queued = true;
					}
				}
			} else if (mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_FORGE) == 0)
			{
				// need to build the forge
				std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_FORGE, 1, true)->execute(aeolusbot);
			}
		}

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		const std::map<::sc2::UNIT_TYPEID, float> armyComp(m_plan.army_composition.begin(), m_plan.army_composition.end());
		aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(armyComp));
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(armyComp));
		aeolusbot.RegisterBehavior(std::make_unique<BuildGeysers>());

		// expanding has the least priority
		aeolusbot.RegisterBehavior(std::make_unique<Expand>());

		if (mediator.GetMinerals(aeolusbot) > 250)
		{
			aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(
				mediator.GetOwnReadyTownHalls(aeolusbot).size() * 22)
			);
		}

		if (aeolusbot.Observation()->GetFoodUsed() > m_plan.move_out_supply)
		{
			aeolusbot.ChangeState(MakeState<ForwardPressureState>());
		}
	}
}