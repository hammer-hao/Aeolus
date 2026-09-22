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
#include "../behaviors/micro_behaviors/micro_behavior.h"
#include "../behaviors/micro_behaviors/attack_target_unit.h"
#include "../behaviors/micro_behaviors/path_to_target.h"
#include "../behaviors/micro_behaviors/keep_unit_safe.h"
#include "consolidate.h"
#include "../utils/unit_utils.h"

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

		// if against a terran and they are proxying, see if we can kill the scv before it finishes
		if (!m_scv_killer_queued && mediator.getOpponentRace(aeolusbot) == ::sc2::Race::Terran)
		{
			auto enemyStructures = mediator.GetAllEnemyStructures(aeolusbot);
			::sc2::Point2D enemyStart = mediator.GetExpansionLocations(aeolusbot).back();
			for (const auto& structure : enemyStructures)
			{
				if (sc2::DistanceSquared2D(structure->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f ||
					(sc2::DistanceSquared2D(structure->pos, enemyStart) > 5000.0f))
				{
					// is a proxy
					if (structure->build_progress >= 1.0f) continue;
					auto workerCandidate = mediator.SelectWorkerClosestTo(aeolusbot, structure->pos);
					if (!workerCandidate.has_value()) continue;
					const ::sc2::Unit* worker = workerCandidate.value();
					float pathDistance = aeolusbot.Query()->PathingDistance(worker, structure->pos);
					{
						auto& typedata = aeolusbot.Observation()->GetUnitTypeData();
						float speed = typedata[worker->unit_type].movement_speed;
						float framesNeeded = (pathDistance / speed) * 16;

						float totalBuildTime = typedata[structure->unit_type].build_time;
						float buildTimeLeft = totalBuildTime * (1 - structure->build_progress);
						float timeLeftForKillingSCV = buildTimeLeft - framesNeeded;
						if (timeLeftForKillingSCV < 105) continue;

						::sc2::Units allWorkers = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::GATHERING);
						if (allWorkers.size() < 2) continue;

						auto worker1 = mediator.SelectWorkerClosestTo(aeolusbot, structure->pos).value();
						auto worker2 = mediator.SelectWorkerClosestTo(aeolusbot, structure->pos).value();
						mediator.AssignRole(aeolusbot, worker1, constants::UnitRole::SCV_KILLER);
						mediator.AssignRole(aeolusbot, worker2, constants::UnitRole::SCV_KILLER);
						m_scv_killer_queued = true;
						break;
					};
				}
			}
		}

		_doSCVKillerMicro(aeolusbot);

		// during the build order, we generally want to defend. However, we would still like to move out
		// if we have enought supply
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		if (!forces.empty())
		{
			int baseToDefend = mediator.getOpponentRace(aeolusbot) == ::sc2::Race::Zerg ? 1 : 0;
			::sc2::Point2D target = mediator.GetDefenseTarget(aeolusbot, baseToDefend);
			doGeneralMicro(aeolusbot, forces, target);
		}

		// Enable prism pick up during contingency if one has been created (not likely)
		doPrismPickUpMicro(aeolusbot);

		// We are open to doing Oracle harass during the contingency stage
		// doOracleDefensiveMicro(aeolusbot);

		// Perform Adept Harassment Micro
		doAdeptHarassMicro(aeolusbot);
	}

	void ContingencyState::_doSCVKillerMicro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		auto scvKillers = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::SCV_KILLER);
		if (scvKillers.empty()) return;

		std::vector<::sc2::Point2D> starting_points;
		for (const auto& unit : scvKillers)
		{
			starting_points.push_back(unit->pos);
		}

		std::vector<::sc2::Units> closeEnemies = mediator.GetEnemyUnitsInRangeMap(aeolusbot, starting_points, 12);
		std::set<::sc2::Tag> seen;
		::sc2::Units enemySCVs;

		for (const auto& group : closeEnemies)
		{
			for (const auto* enemy : group)
			{
				if (enemy->unit_type == ::sc2::UNIT_TYPEID::TERRAN_SCV &&
					seen.insert(enemy->tag).second)
				{
					enemySCVs.push_back(enemy);
				}
			}
		}

		for (const auto& scvKiller : scvKillers)
		{
			auto combat_behavior = std::make_unique<MicroBehavior>(scvKiller);

			if (enemySCVs.empty())
			{
				::sc2::Point2D target = mediator.GetAtttackTarget(aeolusbot);
				::sc2::Point2D enemyStart = mediator.GetExpansionLocations(aeolusbot).back();
				if (sc2::DistanceSquared2D(target, aeolusbot.Observation()->GetStartLocation()) > 5000.0f &&
					(sc2::DistanceSquared2D(target, enemyStart) <= 5000.0f))
				{
					_releaseSCVKillers(aeolusbot);
					aeolusbot.ChangeState(MakeState<ConsolidateState>());
				}
				else
				{
					if (::sc2::Distance2D(scvKiller->pos, target) < 9.0f)
					{
						combat_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
					}
					else
					{
						combat_behavior->AddBehavior(std::make_unique<PathToTarget>(target));
					}
				}
			}
			else
			{
				const ::sc2::Unit* toTarget = utils::PickAttackTarget(enemySCVs);
				combat_behavior->AddBehavior(std::make_unique<AttackTargetUnit>(
					toTarget
				));
			}
			aeolusbot.RegisterBehavior(std::move(combat_behavior));
		}

	}

	void ContingencyState::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		doBookKeepingMacroTasks(aeolusbot);

		if (!m_build_defense_queued)
		{
			const int base_location = mediator.getOpponentRace(aeolusbot) == ::sc2::Race::Zerg ? 1 : 0;
			if (m_plan.batteries_to_add == 0)
			{
				m_build_defense_queued = true;
			}
			else
			{
				bool stillBuildingCyberCore = false;
				if (mediator.IsStructureAvailable(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE))
				{
					for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
					{
						if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE &&
							structure->build_progress > 0.9f)
						{
							for (int i = 0; i < m_plan.batteries_to_add; ++i)
							{
								const ::sc2::UNIT_TYPEID to_build = ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY;
								const bool is_wall = true;
								aeolusbot.RegisterBehavior(std::make_unique<BuildStructure>(to_build, base_location, is_wall));
							}
							m_build_defense_queued = true;
						}
					}
				}
				else if (mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) == 0)
				{
					// need to build the cyber core
					std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, base_location, true)->execute(aeolusbot);
				}
			}
		}

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		const std::map<::sc2::UNIT_TYPEID, float> armyComp(m_plan.army_composition.begin(), m_plan.army_composition.end());
		aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(armyComp, false));
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

		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
		if (forces.size() > 4)
		{
			_releaseSCVKillers(aeolusbot);
			aeolusbot.ChangeState(MakeState<ConsolidateState>());
		}
		else if (aeolusbot.Observation()->GetFoodUsed() > m_plan.move_out_supply)
		{
			_releaseSCVKillers(aeolusbot);
			aeolusbot.ChangeState(MakeState<ForwardPressureState>());
		}
	}

	void ContingencyState::_releaseSCVKillers(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units toRelease = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::SCV_KILLER);

		for (const auto& unit : toRelease)
		{
			mediator.AssignRole(aeolusbot, unit, constants::UnitRole::GATHERING);
		}
	}
}