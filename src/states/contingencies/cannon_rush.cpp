#include "cannon_rush.h"

#include "../contingency.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"

#include "../../behaviors/macro_behaviors/mining.h"
#include "../../behaviors/macro_behaviors/build_workers.h"
#include "../../behaviors/macro_behaviors/chrono_controller.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"
#include "../../behaviors/macro_behaviors/spawn_controller.h"
#include "../../behaviors/macro_behaviors/production_controller.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"
#include "../../behaviors/macro_behaviors/auto_supply.h"
#include "../../behaviors/macro_behaviors/tech_up.h"
#include "../../behaviors/macro_behaviors/scout.h"

#include "../../behaviors/micro_behaviors/micro_behavior.h"
#include "../../behaviors/micro_behaviors/micro_maneuver.h"
#include "../../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../../behaviors/micro_behaviors/attack_target_unit.h"

#include "../../constants.h"

#include "../consolidate.h"

namespace Aeolus
{
	std::string_view CannonRush::getName() const
	{
		return "CANNON_RUSH";
	}

	void CannonRush::micro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		int baseToDefend = 0; // defend the main base
		::sc2::Point2D target = mediator.GetDefenseTarget(aeolusbot, baseToDefend);
		doGeneralMicro(aeolusbot, forces, target);

		for (const auto& [target, probes] : m_pulled_probes)
		{
			for (const auto& probe : probes)
			{
				auto combat_behavior = std::make_unique<MicroBehavior>(probe);

				if ((probe->shield / probe->shield_max) < 0.1)
				{
					combat_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
				}

				auto unitTarget = aeolusbot.Observation()->GetUnit(target);
				combat_behavior->AddBehavior(std::make_unique<AttackTargetUnit>(unitTarget));

				aeolusbot.RegisterBehavior(std::move(combat_behavior));
			}
		}
	}

	void CannonRush::macro(AeolusBot& aeolusbot)
	{
		pullProbes(aeolusbot);
		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<Scout>());
		// prioritize unit production
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(m_army_comp));
		aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(m_army_comp));

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(22));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

		// check if we are safe
		if (aeolusbot.Observation()->GetGameLoop() % 22 != 1) return;
		auto allEnemy = ManagerMediator::getInstance().GetAllEnemyStructures(aeolusbot);
		bool structureLeft = std::any_of(allEnemy.begin(), allEnemy.end(), [&](const ::sc2::Unit* enemy) {
			return (enemy->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON ||
				enemy->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON) &&
				::sc2::DistanceSquared2D(enemy->pos, aeolusbot.Observation()->GetStartLocation()) < 5000;
			});
		if (!structureLeft) {
			unpullAllProbes(aeolusbot);
			aeolusbot.ChangeState(MakeState<ConsolidateState>());
		}
	}

	void CannonRush::pullProbes(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units enemyStructures = mediator.GetAllEnemyStructures(aeolusbot);

		auto& unitData = aeolusbot.Observation()->GetUnitTypeData();
		float probeMovementSpeed =
			unitData[(int)::sc2::UNIT_TYPEID::PROTOSS_PROBE].movement_speed; // tiles per 16 loops
		float pylonBuildTime = unitData[(int)::sc2::UNIT_TYPEID::PROTOSS_PYLON].build_time;
		float cannonBuildTime = unitData[(int)::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON].build_time;
		probeMovementSpeed = probeMovementSpeed / 16;
		for (const auto& structure : enemyStructures)
		{
			if ((structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON ||
				structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON) &&
				m_pulled_probes.find(structure->tag) == m_pulled_probes.end())
			{
				if (structure->build_progress >= 1.0f)
				{
					m_pulled_probes.insert({ structure->tag, {} });
					break;
				}

				// calculate the time needed to approach the structure
				float extraFramesNeeded =
					::sc2::Distance2D(aeolusbot.Observation()->GetStartLocation(), structure->pos) / probeMovementSpeed;
				extraFramesNeeded += 22.4; // add a second to ensure safety
				float structureBuildTime = structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON ?
					cannonBuildTime : pylonBuildTime;
				float structureBuildFramesLeft = structureBuildTime * (1 - structure->build_progress) - extraFramesNeeded;

				if (structureBuildFramesLeft <= 0.0f)
				{
					m_pulled_probes.insert({ structure->tag, {} });
					break;
				}

				int structureTotalHealth = structure->health_max + structure->shield_max;
				// 1 probe = 4 dps = 0.2232 damage per frame
				float damageSingleProbe = structureBuildFramesLeft * 0.17857;
				int probesNeeded = std::ceil(structureTotalHealth / damageSingleProbe);
				if (probesNeeded > 5)
				{
					m_pulled_probes.insert({ structure->tag, {} });
					break;
				}

				std::optional<::sc2::Units> probesAvailable =
					mediator.SelectWorkersClosestTo(aeolusbot, structure->pos, probesNeeded);

				if (!probesAvailable)
				{
					continue;
				}
				else
				{
					m_pulled_probes.insert({ structure->tag,
						std::unordered_set<const ::sc2::Unit*>(probesAvailable.value().begin(), probesAvailable.value().end())
						});
					for (const auto& probe : probesAvailable.value())
					{
						mediator.AssignRole(aeolusbot, probe, constants::UnitRole::WORKER_SOLDIERS);
					}
				}
				break;
			}
		}
	}

	void CannonRush::unpullAllProbes(AeolusBot& aeolusbot)
	{
		for (const auto& [target, probes] : m_pulled_probes)
		{
			for (const auto& probe : probes)
			{
				ManagerMediator::getInstance().AssignRole(aeolusbot, probe, constants::UnitRole::GATHERING);
			}
		}
		m_pulled_probes.clear();
	}

	void CannonRush::unpullProbes(AeolusBot& aeolusbot, ::sc2::Tag target)
	{
		auto it = m_pulled_probes.find(target);
		if (it == m_pulled_probes.end()) return;
		for (const auto& probe : it->second)
		{
			ManagerMediator::getInstance().AssignRole(aeolusbot, probe, constants::UnitRole::GATHERING);
		}
		m_pulled_probes.erase(it);
	}

	void CannonRush::OnUnitDestroyed(
		AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		// If the destroyed unit was a target, release its probes.
		unpullProbes(aeolusbot, unit->tag);

		// If it was a pulled probe, remove it from all assignments.
		for (auto& [target, probes] : m_pulled_probes)
		{
			for (auto it = probes.begin(); it != probes.end();)
			{
				if ((*it)->tag == unit->tag)
					it = probes.erase(it);
				else
					++it;
			}
		}
	}
}