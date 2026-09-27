#pragma once

#include "terran_proxy.h"

#include "../contingency.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"

#include "../../behaviors/macro_behaviors/mining.h"
#include "../../behaviors/macro_behaviors/scout.h"
#include "../../behaviors/macro_behaviors/production_controller.h"
#include "../../behaviors/macro_behaviors/auto_supply.h"
#include "../../behaviors/macro_behaviors/build_workers.h"
#include "../../behaviors/macro_behaviors/spawn_controller.h"
#include "../../behaviors/macro_behaviors/chrono_controller.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"
#include "../../behaviors/macro_behaviors/build_structure.h"
#include "../../behaviors/micro_behaviors/micro_behavior.h"
#include "../../behaviors/micro_behaviors/path_to_target.h"
#include "../../behaviors/micro_behaviors/attack_target_unit.h"
#include "../../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../../behaviors/micro_behaviors/a_move.h"
#include "../../behaviors/macro_behaviors/build_geysers.h"
#include "../../utils/unit_utils.h"
#include "../../states/consolidate.h"
#include "../../states/build_order_state.h"

namespace Aeolus
{
	std::string_view TerranProxy::getName() const
	{
		return "TERRAN_PROXY";
	}

	void TerranProxy::micro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		int baseToDefend = 1; // defend the natural base as terran opponents often do bunker contains
		::sc2::Point2D target = mediator.GetAtttackTarget(aeolusbot);

		_doSCVKillerMicro(aeolusbot);
		doGeneralMicro(aeolusbot, forces, target);
	}

	void TerranProxy::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();

		// see if we can kill the scv before it finishes
		auto enemyStructures = mediator.GetAllEnemyStructures(aeolusbot);
		::sc2::Point2D enemyStart = mediator.GetExpansionLocations(aeolusbot).back();
		if (!m_scv_killer_queued)
		{
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

					auto& typedata = aeolusbot.Observation()->GetUnitTypeData();
					float speed = typedata[worker->unit_type].movement_speed;
					float framesNeeded = (pathDistance / speed) * 16;

					float totalBuildTime = typedata[structure->unit_type].build_time;
					float buildTimeLeft = totalBuildTime * (1 - structure->build_progress);
					float timeLeftForKillingSCV = buildTimeLeft - framesNeeded;
					if (timeLeftForKillingSCV < 105) continue;

					std::optional<::sc2::Units> pulledWorkers = mediator.SelectWorkersClosestTo(aeolusbot, structure->pos, 2);
					if (!pulledWorkers) continue;
					for (const auto& worker : pulledWorkers.value())
					{
						mediator.AssignRole(aeolusbot, worker, constants::UnitRole::SCV_KILLER);
					}
					m_scv_killer_queued = true;
					break;
				}
			}
		}

		// build a shield battery at natural
		auto ownStructure = mediator.GetAllOwnStructures(aeolusbot);
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
		// prioritize unit production
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(m_army_comp));
		aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(m_army_comp, false));

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(22));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());
		aeolusbot.RegisterBehavior(std::make_unique<BuildGeysers>());

		// check if we are safe
		if (aeolusbot.Observation()->GetGameLoop() % 22 != 1) return;
		auto allEnemy = ManagerMediator::getInstance().GetAllEnemyStructures(aeolusbot);
		bool structureLeft = std::any_of(allEnemy.begin(), allEnemy.end(), [&](const ::sc2::Unit* enemy) {
			return (enemy->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BARRACKS ||
				enemy->unit_type == ::sc2::UNIT_TYPEID::TERRAN_FACTORY ||
				enemy->unit_type == ::sc2::UNIT_TYPEID::TERRAN_STARPORT ||
				enemy->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BUNKER) &&
				::sc2::DistanceSquared2D(enemy->pos, aeolusbot.Observation()->GetStartLocation()) < 5000;
			});
		if (!structureLeft) {
			_releaseSCVKillers(aeolusbot);
			aeolusbot.ChangeState(MakeState<BuildOrderState>());
		}
	}

	void TerranProxy::_doSCVKillerMicro(AeolusBot& aeolusbot)
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
				}
				else
				{
					if (::sc2::Distance2D(scvKiller->pos, target) < 9.0f)
					{
						combat_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
						combat_behavior->AddBehavior(std::make_unique<AMove>(target));
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

	void TerranProxy::_releaseSCVKillers(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units toRelease = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::SCV_KILLER);

		for (const auto& unit : toRelease)
		{
			mediator.AssignRole(
				aeolusbot,
				unit,
				constants::UnitRole::GATHERING);

			// Cancel command left over from SCV_KILLER role.
			aeolusbot.Actions()->UnitCommand(
				unit,
				::sc2::ABILITY_ID::STOP);
		}
	}
}