#include "proxy_pylon.h"

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
#include "../../behaviors/macro_behaviors/repower_structures.h"

#include "../../behaviors/micro_behaviors/micro_behavior.h"
#include "../../behaviors/micro_behaviors/a_move.h"
#include "../../behaviors/micro_behaviors/attack_target_unit.h"
#include "../../behaviors/micro_behaviors/keep_unit_safe.h"

#include "cannon_rush.h"
#include "proxy_gateways.h"

namespace Aeolus
{
	std::string_view ProxyPylon::getName() const
	{
		return "PROXY_PYLON";
	}

	void ProxyPylon::micro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);

		int baseToDefend = 0; // defend the main base
		::sc2::Point2D target = mediator.GetDefenseTarget(aeolusbot, baseToDefend);
		doGeneralMicro(aeolusbot, forces, target);

		// scout around the pylon
		if (m_scout)
		{
			auto scoutTarget = mediator.GetAtttackTarget(aeolusbot);
			auto combat_behavior = std::make_unique<MicroBehavior>(m_scout);

			if (::sc2::Distance2D(m_scout->pos, scoutTarget) < 9.0f)
			{
				combat_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
			}
			else
			{
				combat_behavior->AddBehavior(std::make_unique<AMove>(scoutTarget));
			}

			aeolusbot.RegisterBehavior(std::move(combat_behavior));
		}
	}

	void ProxyPylon::macro(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		auto allStructures = mediator.GetAllOwnStructures(aeolusbot);
		std::map<::sc2::UNIT_TYPEID, float> army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0} };

		if (!m_scout_sent)
		{
			auto closestWorker = mediator.SelectWorkerClosestTo(aeolusbot, mediator.GetAtttackTarget(aeolusbot));
			if (closestWorker)
			{
				mediator.AssignRole(aeolusbot, closestWorker.value(), constants::UnitRole::SCV_KILLER);
				m_scout = closestWorker.value();
				m_scout_sent = true;
			}
		}

		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<Scout>());
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(army_comp));
		aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(army_comp));

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());
		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(22));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

		::sc2::Units allEnemy = mediator.GetAllSeenEnemyUnits(aeolusbot);
		::sc2::Units allEnemyStructures = mediator.GetAllEnemyStructures(aeolusbot);
		for (const auto& structure : allEnemyStructures)
		{
			allEnemy.push_back(structure);
		}
		const ::sc2::Point2D enemyStartLocation = ManagerMediator::getInstance().GetExpansionLocations(aeolusbot).back();
		int gameLoop = aeolusbot.Observation()->GetGameLoop();

		if (aeolusbot.Observation()->GetGameLoop() % 22 == 1)
		{
			if (gameLoop < 2688)
			{
				for (const auto& unit : allEnemy)
				{
					if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON &&
						sc2::DistanceSquared2D(unit->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f)
					{
						sendChatTag(aeolusbot, "cannon_rush");
						if (m_scout)
						{
							mediator.AssignRole(
								aeolusbot, m_scout, constants::UnitRole::GATHERING);
							m_scout = nullptr;
						}
						aeolusbot.ChangeState(MakeState<CannonRush>());
						return;
					}
					if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY &&
						(sc2::DistanceSquared2D(unit->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f ||
							sc2::DistanceSquared2D(unit->pos, enemyStartLocation) > 5000.0f))
					{
						sendChatTag(aeolusbot, "proxy_gateway");
						if (m_scout)
						{
							mediator.AssignRole(
								aeolusbot, m_scout, constants::UnitRole::GATHERING);
							m_scout = nullptr;
						}
						aeolusbot.ChangeState(MakeState<ProxyGateways>());
						return;
					}
				}
			}

			const int numGateways = 3;
			auto ownAttacking = ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
			if (ownAttacking.size() >= numGateways)
			{
				if (m_scout)
				{
					mediator.AssignRole(
						aeolusbot, m_scout, constants::UnitRole::GATHERING);
					m_scout = nullptr;
				}
				aeolusbot.ChangeState(MakeState<ConsolidateState>());
			}
		}
	}

	void ProxyPylon::OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
	{
		if (m_scout && unit->tag == m_scout->tag)
		{
			m_scout = nullptr;
		}
	}
}