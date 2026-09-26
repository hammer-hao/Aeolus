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

#include "contingencies/worker_rush.h"
#include "contingencies/cannon_rush.h"
#include "contingencies/proxy_gateways.h"
#include "contingencies/terran_proxy.h"
#include "contingencies/one_base_marines.h"
#include "contingencies/proxy_pylon.h"
#include "contingencies/twelve_pool.h"
#include "contingencies/marauder_rush.h"

namespace Aeolus
{
	std::string_view ContingencyState::getName() const
	{
		return "CONTINGENCY";
	}

	bool ContingencyState::ensureContingencyResponse(AeolusBot& aeolusbot)
	{
		if (aeolusbot.Observation()->GetGameLoop() % 10 != 1) return false;

		::sc2::Race opponentRace = ManagerMediator::getInstance().getOpponentRace(aeolusbot);

		if (opponentRace == ::sc2::Race::Protoss)
		{
			return ensureResponseAgainstProtoss(aeolusbot);
		}
		else if (opponentRace == ::sc2::Race::Terran)
		{
			return ensureResponseAgainstTerran(aeolusbot);
		}
		else if (opponentRace == ::sc2::Race::Zerg)
		{
			return ensureResponseAgainstZerg(aeolusbot);
		}
		else
		{
			if (ensureResponseAgainstProtoss(aeolusbot)) return true;
			if (ensureResponseAgainstTerran(aeolusbot)) return true;
			if (ensureResponseAgainstZerg(aeolusbot)) return true;
			return false;
		}
	}

	bool ContingencyState::ensureResponseAgainstProtoss(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		auto& unitTypes = aeolusbot.Observation()->GetUnitTypeData();
		::sc2::Units allEnemy = mediator.GetAllSeenEnemyUnits(aeolusbot);
		int gameLoop = aeolusbot.Observation()->GetGameLoop();

		const ::sc2::Point2D enemyStartLocation = ManagerMediator::getInstance().GetExpansionLocations(aeolusbot).back();
		int seenProbes = 0;

		for (const auto& unit : allEnemy)
		{
			if (gameLoop < 2688)
			{
				if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON &&
					sc2::DistanceSquared2D(unit->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f)
				{
					sendChatTag(aeolusbot, "cannon_rush");
					aeolusbot.ChangeState(MakeState<CannonRush>());
					return true;
				}
				if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY &&
					(sc2::DistanceSquared2D(unit->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f ||
						sc2::DistanceSquared2D(unit->pos, enemyStartLocation) > 5000.0f))
				{
					sendChatTag(aeolusbot, "proxy_gateway");
					aeolusbot.ChangeState(MakeState<ProxyGateways>());
					return true;
				}
				if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON && (sc2::DistanceSquared2D(unit->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f ||
					sc2::DistanceSquared2D(unit->pos, enemyStartLocation) > 5000.0f))
				{
					sendChatTag(aeolusbot, "proxy_pylon");
					aeolusbot.ChangeState(MakeState<ProxyPylon>());
					return true;
				}
				if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PROBE) seenProbes++;
			}
		}
		if (seenProbes >= 8 && gameLoop < (22.4 * 60))
		{
			sendChatTag(aeolusbot, "worker_rush");
			sendChatTag(aeolusbot, "probe_rush");

			aeolusbot.ChangeState(MakeState<WorkerRush>());
			return true;
		}
		return false;
	}

	bool ContingencyState::ensureResponseAgainstTerran(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		auto& unitTypes = aeolusbot.Observation()->GetUnitTypeData();
		::sc2::Units allEnemy = mediator.GetAllSeenEnemyUnits(aeolusbot);
		int gameLoop = aeolusbot.Observation()->GetGameLoop();

		const ::sc2::Point2D enemyStartLocation = ManagerMediator::getInstance().GetExpansionLocations(aeolusbot).back();
		int seenSCVs = 0;
		int seenMarines = 0;
		int seenCCs = 0;
		int seenBarracks = 0;
		int seenFactories = 0;
		bool seenTerranNatural = false;
		::sc2::Point2D enemyNaturalPos = mediator.GetEnemyNaturalPosition(aeolusbot);
		for (const auto& unit : allEnemy)
		{
			if (gameLoop < 2688)
			{
				if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BARRACKS ||
					unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_FACTORY ||
					unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BUNKER)
				{
					if (sc2::DistanceSquared2D(unit->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f ||
						(sc2::DistanceSquared2D(unit->pos, enemyStartLocation) > 5000.0f))
					{
						sendChatTag(aeolusbot, "terran_proxy");
						aeolusbot.ChangeState(MakeState<TerranProxy>());
						return true;
					}
				}
			}
			if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_SCV)
			{
				seenSCVs++;
			}
			if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_MARINE)
			{
				seenMarines++;
			}
			if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER
				|| unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND
				|| unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS)
			{
				seenCCs++;
				if (sc2::DistanceSquared2D(unit->pos, enemyNaturalPos) < 9)
				{
					seenTerranNatural = true;
				}
			}
			if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BARRACKS)
			{
				seenBarracks++;
			}
			if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_FACTORY)
			{
				seenFactories++;
			}
			if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_MARAUDER)
			{
				if (sc2::DistanceSquared2D(unit->pos, aeolusbot.Observation()->GetStartLocation()) < 5000.0f &&
					gameLoop < (22.4 * 180))
				{
					sendChatTag(aeolusbot, "marauder_rush");
					aeolusbot.ChangeState(MakeState<MarauderRush>());
					return true;
				}
			}
		}

		if (seenSCVs >= 8 && gameLoop < (22.4 * 60))
		{
			sendChatTag(aeolusbot, "worker_rush");
			sendChatTag(aeolusbot, "scv_rush");
			aeolusbot.ChangeState(MakeState<WorkerRush>());
			return true;
		}

		if (seenMarines >= 4 && !seenTerranNatural && gameLoop < (22.4 * 180))
		{
			sendChatTag(aeolusbot, "terran_one_base_marines");
			aeolusbot.ChangeState(MakeState<OneBaseMarines>());
			return true;
		}
		//if (seenBarracks >= 2 && !seenTerranNatural)
		//{
		//	sendChatTag(aeolusbot, "terran_one_base_bio");
		//}
		//if (seenFactories > 0 && !seenTerranNatural)
		//{
		//	sendChatTag(aeolusbot, "terran_one_base");
		//}

		return false;
	}

	bool ContingencyState::ensureResponseAgainstZerg(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		::sc2::Units allEnemy = mediator.GetAllSeenEnemyUnits(aeolusbot);
		int dronesSeen = 0;
		int gameLoop = aeolusbot.Observation()->GetGameLoop();
		for (const auto& unit : allEnemy)
		{
			if (unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_SPAWNINGPOOL)
			{
				float buildTime = aeolusbot.Observation()->GetUnitTypeData()[unit->unit_type].build_time;
				float buildProgress = unit->build_progress;
				float timeSpentBuilding = buildTime * buildProgress;
				float startedAt = gameLoop - timeSpentBuilding;

				if (startedAt < 650)
				{
					sendChatTag(aeolusbot, "12_pool");
					aeolusbot.ChangeState(MakeState<TwelvePool>());
					return true;
				}

				//if (startedAt < 1150)
				//{
				//	sendChatTag(aeolusbot, "pool_first");
				//	break;
				//}
			}
			if (unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_DRONE) dronesSeen++;
		}

		if (dronesSeen >= 8 && gameLoop < (22.4 * 60))
		{
			sendChatTag(aeolusbot, "worker_rush");
			sendChatTag(aeolusbot, "drone_rush");
			aeolusbot.ChangeState(MakeState<WorkerRush>());
			return true;
		}
		return false;
	}

	void ContingencyState::sendChatTag(AeolusBot& aeolusbot, std::string to_send)
	{
		std::stringstream scoutedTag;
		scoutedTag << "Tag:";
		scoutedTag << to_send;
		aeolusbot.Actions()->SendChat(scoutedTag.str());
	}
}