#include <iostream>
#include <unordered_map>
#include <sc2api/sc2_api.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_action.h>

#include "../../managers/manager_mediator.h"
#include "../../constants.h"

#include "mining.h"
#include "../../utils/position_utils.h"
#include "../../utils/unit_utils.h"
#include "../../utils/game_utils.h"
#include "../../Aeolus.h"

namespace Aeolus
{
	void Mining::execute(AeolusBot& aeolusbot)
	{
		
		// std::cout << "<<< Mining Behavior: Executing... >>>" << std::endl;
		::sc2::Units workers = ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::GATHERING);
		std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*> worker_to_patch = ManagerMediator::getInstance().GetWorkersToPatch(aeolusbot);

		m_patch_map = ManagerMediator::getInstance().GetMineralGatheringPoints(aeolusbot);

		/*
		#ifdef BUILD_WITH_RENDERER

		auto* debug = aeolusbot.Debug();

		auto height_map = ::sc2::HeightMap(aeolusbot.Observation()->GetGameInfo());

		for (const auto& point : m_patch_map)
		{
			debug->DebugSphereOut(::sc2::Point3D(point.second.x, point.second.y, height_map.TerrainHeight(point.second)), 0.25, ::sc2::Colors::White);
			debug->DebugLineOut(::sc2::Point3D(point.second.x, point.second.y, height_map.TerrainHeight(point.second)),
				::sc2::Point3D(point.first.first, point.first.second, height_map.TerrainHeight(::sc2::Point2D(point.first.first, point.first.second))),
				::sc2::Colors::Red);
		}

		#endif

		*/

		m_town_halls = ManagerMediator::getInstance().GetOwnTownHalls(aeolusbot);

		::sc2::Units ground_threats = ManagerMediator::getInstance().GetGroundThreatsNearBases(aeolusbot);

		// std::cout << "mining: got " << ground_threats.size() << " ground threats" << std::endl;

		for (const auto worker : workers)
		{
			double distance_to_resource = 15.0;
			bool assigned_to_resource = (worker_to_patch.find(worker) != worker_to_patch.end());
			if (assigned_to_resource)
				distance_to_resource = ::sc2::Distance2D(
					::sc2::Point2D(worker->pos), 
					::sc2::Point2D(worker_to_patch[worker]->pos));

			double percentage_health = (m_self_race == ::sc2::Race::Protoss) ? 
				worker->shield / worker->shield_max : worker->health / worker->health_max;

			if (m_keep_safe && percentage_health < m_flee_at_health_perc)
			{
				bool is_position_safe = ManagerMediator::getInstance().IsGroundPositionSafe(aeolusbot, ::sc2::Point2D(worker->pos));
				if (!is_position_safe) std::cout << "Position is not safe and health critical. escaping..." << std::endl;
				::sc2::Point2D safe_spot = ManagerMediator::getInstance().FindClosestGroundSafeSpot(aeolusbot, ::sc2::Point2D(worker->pos), 7.0);
				aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::MOVE_MOVE, safe_spot);
			}
			else if (!ground_threats.empty() && _workerIsAttacking(aeolusbot, worker, ground_threats, distance_to_resource)) // detected threats near the town hall
			{
				std::cout << "Drilling enemy! " << std::endl;
				continue;
			}
			else if (assigned_to_resource)
			{
				// ::sc2::Point2D start_location_2d = utils::ConvertTo2D(aeolusbot.Observation()->GetStartLocation());
				//aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, start_location_2d);
				//aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, m_patch_map[worker_to_patch[worker]]);

				if (!worker->orders.empty())
				{
					if (worker->orders[0].ability_id == ::sc2::ABILITY_ID::HARVEST_GATHER
						&& worker->orders[0].target_unit_tag != worker_to_patch[worker]->tag)
					{
						// shift worker to correct resource if it ends up on wrong one
						// std::cout << "Worker in the wrong spot, shifting..." << std::endl;
						aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, worker_to_patch[worker]);
					}
				}
				// debug->DebugLineOut(worker->pos, worker_to_patch[worker]->pos, ::sc2::Colors::Red);

				DoMiningBoost(worker_to_patch[worker], worker, m_patch_map, aeolusbot);
			}
		}
	}

	void Mining::DoMiningBoost(
		const ::sc2::Unit* patch,
		const ::sc2::Unit* worker,
		const std::map<std::pair<float, float>, ::sc2::Point2D>& patch_target_map,
		AeolusBot& aeolusbot)
	{
		/*
		Perform the trick so that worker does not decelerate.

        This avoids worker deceleration when mining by issuing a Move command near a
        mineral patch/townhall and then issuing a Gather or Return command once the
        worker is close enough to immediately perform the action instead of issuing a
        Gather command and letting the SC2 engine manage the worker.
		
		*/
		// std::cout << "Doing mining boost..." << std::endl;

		// ::sc2::Point2D mineral_move_position = m_patch_map[patch];
		::sc2::Point2D mineral_move_position = m_patch_map[{patch->pos.x, patch->pos.y}];
		::sc2::Point2D worker_position = utils::ConvertTo2D(worker->pos);
		const ::sc2::Unit* closest_town_hall = utils::GetClosestUnitTo(utils::ConvertTo2D(worker->pos), m_town_halls);

		if ((utils::IsWorkerCarryingResource(worker) || utils::HasAbilityQueued(worker, ::sc2::ABILITY_ID::HARVEST_RETURN)) && worker->orders.size() < 2)
		{
			::sc2::Point2D target_position = utils::GetPositionTowards(
				utils::ConvertTo2D(closest_town_hall->pos),
				worker_position,
				constants::TOWNHALL_DISTANCE_FACTOR * constants::TOWNHALL_RADIUS
			);

			if ((constants::MINING_BOOST_MIN_RADIUS < ::sc2::DistanceSquared2D(worker_position, target_position))
				&& (::sc2::DistanceSquared2D(worker_position, target_position)  < constants::MINING_BOOST_MAX_RADIUS))
			{
				// std::cout << "returning..." << std::endl;
				aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::MOVE_MOVE, target_position);
				aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, closest_town_hall, true);
			}
			else if (!utils::HasAbilityQueued(worker, ::sc2::ABILITY_ID::HARVEST_RETURN))
			{
				// worker has resource but for some reason does not have return minerals queued
				aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, closest_town_hall);
			}
		}
		else if (!utils::HasAbilityQueued(worker, ::sc2::ABILITY_ID::HARVEST_RETURN) && worker->orders.size() < 2)
		{
			const auto height_map = ::sc2::HeightMap(aeolusbot.Observation()->GetGameInfo());
			/*
			aeolusbot.Debug()->DebugTextOut(
				std::to_string(::sc2::DistanceSquared2D(worker_position, mineral_move_position)),
				worker->pos);
			aeolusbot.Debug()->DebugLineOut(
				::sc2::Point3D(mineral_move_position.x, mineral_move_position.y, height_map.TerrainHeight(mineral_move_position)),
				worker->pos,
				::sc2::Colors::Teal);
			*/
			if ((constants::MINING_BOOST_MIN_RADIUS < ::sc2::DistanceSquared2D(worker_position, mineral_move_position))
				&& (::sc2::DistanceSquared2D(worker_position, mineral_move_position) < constants::MINING_BOOST_MAX_RADIUS))
				// worker is idle
			{
				// std::cout << ::sc2::DistanceSquared2D(worker_position, mineral_move_position) << std::endl;
				// std::cout << "harvesting..." << std::endl;
				aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::MOVE_MOVE, mineral_move_position);
				aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, patch, true);
			}
			// on rare occations, the worker goes idle
			else if (worker->orders.empty() || utils::HasAbilityQueued(worker, ::sc2::ABILITY_ID::STOP))
			{
				std::cout << "worker is idle!!" << std::endl;
				if (worker->cargo_space_taken > 0)
				{
					aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, closest_town_hall, true);
				}
				else
				{
					aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::SMART, patch, true);
				}
			}
		}
	}

	bool Mining::_workerIsAttacking(AeolusBot& aeolusbot, const ::sc2::Unit* worker, ::sc2::Units targets, double distance_to_resource)
	{
		// std::cout << "distance to resource: " << distance_to_resource << std::endl;
		// attack enemy logic:
		if (!(utils::HasAbilityQueued(worker, ::sc2::ABILITY_ID::HARVEST_GATHER)
			|| utils::HasAbilityQueued(worker, ::sc2::ABILITY_ID::HARVEST_RETURN))
			|| distance_to_resource > 1.0)
		{
			// std::cout << "Worker ready for attack! checking for units in range... " << std::endl;
			// can attack enemy if worker has nothing to do / is far enough from mineral patch
			::sc2::Units enemies = ManagerMediator::getInstance().GetUnitsInAtttackRange(aeolusbot, worker, targets);
			if (!enemies.empty())
			{
				std::cout << "Attacking!!! " << std::endl;
				const ::sc2::Unit* target = utils::PickAttackTarget(enemies);
				aeolusbot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::ATTACK, target);
				return true;
			}
		}
		return false;
	}
}