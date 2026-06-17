#include "path_manager.h"
#include "../pathing/weight_costs.h"
#include "../Aeolus.h"
#include "../constants.h"
#include "../utils/Astar.hpp"
#include "manager_mediator.h"

#include <sc2api/sc2_common.h>
#include <any>
#include <tuple>

namespace Aeolus
{
	void PathManager::update(int iteration)
	{
		if (iteration == 0)
		{
			m_mapdata.update();
			m_ground_grid = m_mapdata.GetAStarGrid();
			m_prism_grid = m_ground_grid;
			m_air_grid = Grid(m_ground_grid.GetWidth(), m_ground_grid.GetHeight());
			m_ground_grid.UpdateCache();
			m_ground_grid.UpdateCache();
		}
		else if (iteration > 0)
		{
			_reset_grids(); // clean the grids before populating
			_reset_danger_tiles(); // clean the danger tiles cache

			::sc2::Units enemy_units = ManagerMediator::getInstance().GetAllEnemyUnits(m_bot);

			for (const auto unit : enemy_units)
			{
				AddUnitInfluence(unit);
			}

			::sc2::Units enemey_static_defenses = ManagerMediator::getInstance().GetAllEnemyStaticDefenses(m_bot);
			for (const auto unit : enemey_static_defenses)
			{
				AddUnitInfluence(unit);
			}
		}
	}

	std::any PathManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_DEFAULT_GRID_DATA):
		{
			return _getDefaultGridData();
		}
		case (constants::ManagerRequestType::GET_ASTAR_GRID_DATA):
		{
			return _getAStarGrid();
		}
		case (constants::ManagerRequestType::GET_GROUND_GRID):
		{
			return m_ground_grid;
		}
		case (constants::ManagerRequestType::GET_AIR_GRID):
		{
			return m_air_grid;
		}
		case (constants::ManagerRequestType::FIND_CLOSEST_GROUND_SAFE_SPOT):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, double>>(args);
			::sc2::Point2D position = std::get<0>(params);
			double radius = std::get<1>(params);
			return _getClosestSafeSpot(position, radius);
		}
		case (constants::ManagerRequestType::FIND_CLOSEST_AIR_SAFE_SPOT):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, double>>(args);
			::sc2::Point2D position = std::get<0>(params);
			double radius = std::get<1>(params);
			return _getClosestAirSafeSpot(position, radius);
		}
		case (constants::ManagerRequestType::FIND_CLOSEST_PRISM_SAFE_SPOT):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, double>>(args);
			::sc2::Point2D position = std::get<0>(params);
			double radius = std::get<1>(params);
			return _getClosestPrismSafeSpot(position, radius);
		}
		case (constants::ManagerRequestType::FIND_FURTHEST_GROUND_SAFE_SPOT_TOWARDS):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, ::sc2::Point2D, double>>(args);
			::sc2::Point2D position = std::get<0>(params);
			::sc2::Point2D target = std::get<1>(params);
			double radius = std::get<2>(params);
			return _getFuthestGroundSafeSpotTowards(position, target, radius);
		}
		case (constants::ManagerRequestType::FIND_FURTHEST_AIR_SAFE_SPOT_TOWARDS):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, ::sc2::Point2D, double>>(args);
			::sc2::Point2D position = std::get<0>(params);
			::sc2::Point2D target = std::get<1>(params);
			double radius = std::get<2>(params);
			return _getFuthestAirSafeSpotTowards(position, target, radius);
		}
		case (constants::ManagerRequestType::FIND_CLOSEST_SAFE_SPOT_TOWARDS):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, ::sc2::Point2D, double, GridType>>(args);
			::sc2::Point2D position = std::get<0>(params);
			::sc2::Point2D target = std::get<1>(params);
			double radius = std::get<2>(params);
			GridType gridType = std::get<3>(params);
			return _findClosestSafeSpotTowards(position, target, radius, gridType);
		}
		case (constants::ManagerRequestType::IS_GROUND_POSITION_SAFE):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D>>(args);
			::sc2::Point2D position = std::get<0>(params);
			return _isGroundPositionSafe(position);
		}
		case (constants::ManagerRequestType::IS_AIR_POSITION_SAFE):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D>>(args);
			::sc2::Point2D position = std::get<0>(params);
			return _isAirPositionSafe(position);
		}
		case (constants::ManagerRequestType::IS_PRISM_POSITION_SAFE):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D>>(args);
			::sc2::Point2D position = std::get<0>(params);
			return _isPrismPositionSafe(position);
		}
		case (constants::ManagerRequestType::GET_FLOOD_FILL_AREA):
		{
			auto params = std::any_cast<std::tuple <::sc2::Point2D, int>>(args);
			::sc2::Point2D starting_point = std::get<0>(params);
			int max_distance = std::get<1>(params);
			return _getFloodFillArea(starting_point, max_distance);
		}
		case (constants::ManagerRequestType::GET_NEXT_PATH_POINT):
		{
			auto params = std::any_cast<std::tuple <::sc2::Point2D, ::sc2::Point2D, GridType, bool, int, float, bool, int>>(args);
			::sc2::Point2D start = std::get<0>(params);
			::sc2::Point2D goal = std::get<1>(params);
			GridType gridType = std::get<2>(params);
			bool sense_danger = std::get<3>(params);
			int danger_distance = std::get<4>(params);
			float danger_threshold = std::get<5>(params);
			bool smoothing = std::get<6>(params);
			int sensitivity = std::get<7>(params);
			return AStarPathFindNext(start, goal, gridType, sense_danger, danger_distance, danger_threshold, smoothing, sensitivity);
		}
		case (constants::ManagerRequestType::IS_SPOT_SAFER_THAN):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, ::sc2::Point2D, GridType>>(args);
			::sc2::Point2D posA = std::get<0>(params);
			::sc2::Point2D posB = std::get<1>(params);
			GridType gridType = std::get<2>(params);
			return _isSpotSaferThan(posA, posB, gridType);
		}
		default:
			return 0;
		}
	}

	void PathManager::AddUnitInfluence(const ::sc2::Unit* enemy)
	{
		_addUnitInfluence(enemy);
	}

	void PathManager::_addUnitInfluence(const ::sc2::Unit* unit)
	{
		// std::cout << "PathManager: adding unit influence... " << std::endl;
		ManagerMediator& mediator = ManagerMediator::getInstance();

		if (constants::WEIGHT_COSTS.find(unit->unit_type)
			!= constants::WEIGHT_COSTS.end())
		{
			//  std::cout << "PathManager: found existing unit profile! " << std::endl;
			// if we pre-defined unit ground/air weight and range
			auto it = constants::WEIGHT_COSTS.find(unit->unit_type);
			double ground_cost = it->second.GroundCost;
			double ground_range = it->second.GroundRange;

			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer, ground_cost);
			m_prism_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer + 1.0, ground_cost);

			double air_cost = it->second.AirCost;
			double air_range = it->second.AirRange;
			
			m_air_grid.AddCost(unit->pos.x, unit->pos.y, air_range + Config::range_buffer, air_cost);
			m_prism_grid.AddCost(unit->pos.x, unit->pos.y, air_range + Config::range_buffer + 1.0, air_cost);
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_BUNKER)
		{
			// add the range of marines + 1;
			double ground_cost = 20;
			double ground_range = 6;
			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range, ground_cost);
			m_prism_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + 1.0, ground_cost);

			double air_cost = 20;
			double air_Range = 6;
			m_air_grid.AddCost(unit->pos.x, unit->pos.y, air_Range, air_cost);
			m_prism_grid.AddCost(unit->pos.x, unit->pos.y, air_Range + 1.0, air_cost);
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED)
		{
			// A disruptor Nova
			double ground_cost = 1000;
			double ground_range = 8;
			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range, ground_cost);
			m_prism_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer + 1.0, ground_cost);
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_BANELING)
		{
			// A baneling
			// this should already by in the weight_cost dict! monitor if we need to add more logic!
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_INFESTOR && unit->energy >= 75)
		{
			// infestor with fungal
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ORACLE && unit->energy >= 25)
		{
			// oracle with pulsar beam
		}
		
		else if (mediator.CanAttackGround(m_bot, unit))
		{
			// std::cout << "PathManager: found unit with ground attack " << std::endl;
			double ground_range = mediator.GroundRange(m_bot, unit);
			// std::cout << "PathManager: unit range acquired " << std::endl;
			double ground_dps = mediator.GroundDPS(m_bot, unit);
			// std::cout << "PathManager: unit ground dps acquired " << std::endl;
			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer, ground_dps);
			m_prism_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer + 1.0, ground_dps);

			if (ground_range < 2)
			{
				// melee units
			}
			else
			{
				// non-melee units
				// handle ground attack here

				if (mediator.CanAttackAir(m_bot, unit))
				{
					// handle air attack
					double air_range = mediator.AirRange(m_bot, unit);
					double air_dps = mediator.AirDPS(m_bot, unit);

					m_air_grid.AddCost(unit->pos.x, unit->pos.y, air_range + Config::range_buffer, air_dps);
					m_prism_grid.AddCost(unit->pos.x, unit->pos.y, air_range + Config::range_buffer + 1.0, air_dps);
				}
			}
		}
		else if (ManagerMediator::getInstance().CanAttackAir(m_bot, unit))
		{
			// units with air attack only (no attack vs ground)
			// handle air attack
			double air_range = mediator.AirRange(m_bot, unit);
			double air_dps = mediator.AirDPS(m_bot, unit);

			m_air_grid.AddCost(unit->pos.x, unit->pos.y, air_range + Config::range_buffer, air_dps);
		}
	}

	::sc2::Point2D PathManager::_getClosestSafeSpot(::sc2::Point2D position, const double& radius)
	{
		return m_ground_grid.FindClosestSafeSpot(position, radius);
	}

	::sc2::Point2D PathManager::_getClosestAirSafeSpot(::sc2::Point2D position, const double& radius)
	{
		return m_air_grid.FindClosestSafeSpot(position, radius);
	}

	::sc2::Point2D PathManager::_getFuthestGroundSafeSpotTowards(::sc2::Point2D position, ::sc2::Point2D target, const double& radius)
	{
		return m_ground_grid.FindFurthestSafeSpotTowards(position, target, radius);
	}

	::sc2::Point2D PathManager::_getFuthestAirSafeSpotTowards(::sc2::Point2D position, ::sc2::Point2D target, const double& radius)
	{
		return m_air_grid.FindFurthestSafeSpotTowards(position, target, radius);
	}

	::sc2::Point2D PathManager::_getClosestPrismSafeSpot(::sc2::Point2D position, const double& radius)
	{
		return m_prism_grid.FindClosestSafeSpot(position, radius);
	}

	bool PathManager::_isGroundPositionSafe(::sc2::Point2D position)
	{
		return m_ground_grid.IsPositionSafe(position);
	}

	bool PathManager::_isAirPositionSafe(::sc2::Point2D position)
	{
		return m_air_grid.IsPositionSafe(position);
	}

	bool PathManager::_isPrismPositionSafe(::sc2::Point2D position)
	{
		return m_prism_grid.IsPositionSafe(position);
	}

	::sc2::Point2D PathManager::_findClosestSafeSpotTowards(::sc2::Point2D position, ::sc2::Point2D target, double radius, GridType gridType)
	{
		if (gridType == GridType::GROUND)
		{
			return m_ground_grid.FindClosestSafeSpotTowards(position, target, radius);
		}
		else if (gridType == GridType::AIR)
		{
			return m_ground_grid.FindClosestSafeSpotTowards(position, target, radius);
		}
		else if (gridType == GridType::BOTH)
		{
			return m_ground_grid.FindClosestSafeSpotTowards(position, target, radius);
		}
		return m_ground_grid.FindClosestSafeSpotTowards(position, target, radius);
	}

	bool PathManager::_isSpotSaferThan(::sc2::Point2D posA, ::sc2::Point2D posB, GridType gridType)
	{
		if (gridType == GridType::GROUND)
		{
			return m_ground_grid.isSpotSaferThan(posA, posB);
		}
		else if (gridType == GridType::AIR)
		{
			return m_air_grid.isSpotSaferThan(posA, posB);
		}
		else if (gridType == GridType::BOTH)
		{
			return m_prism_grid.isSpotSaferThan(posA, posB);
		}
		return m_ground_grid.isSpotSaferThan(posA, posB);
	}

	void PathManager::_reset_grids()
	{
		m_ground_grid.Reset();
		m_air_grid.Reset();
	}

	void PathManager::_reset_danger_tiles()
	{
		m_danger_tiles_cache.clear();
		m_danger_tiles_is_cached = false;
	}

	::sc2::ImageData PathManager::_getDefaultGridData()
	{
		return m_mapdata.getDefaultGridData();
	}

	Grid PathManager::_getAStarGrid()
	{
		return m_mapdata.GetAStarGrid();
	}

	std::vector<::sc2::Point2D> PathManager::_getFloodFillArea(::sc2::Point2D starting_point, int max_distance)
	{
		return m_mapdata.GetFloodFillArea(starting_point, max_distance);
	}

	::sc2::Point2D PathManager::AStarPathFindNext(::sc2::Point2D start, ::sc2::Point2D goal,
		GridType gridType, bool sense_danger, int danger_distance,
		float danger_threshold, bool smoothing, int sensitivity)
	{
		Grid& avoidanceGrid = m_ground_grid;
		
		if (gridType == GridType::AIR) avoidanceGrid = m_air_grid;
		else if (gridType == GridType::BOTH) avoidanceGrid = m_prism_grid; // gridType == GridType::BOTH

		const auto& cost_grid = avoidanceGrid.GetGrid();

		if (sense_danger)
		{
			std::vector<std::pair<int, int>> dangers;

			if (m_danger_tiles_is_cached) dangers = m_danger_tiles_cache;
			else
			{
				for (int y = 0; y < cost_grid.rows(); ++y)
				{
					for (int x = 0; x < cost_grid.cols(); ++x)
					{
						if (cost_grid(y, x) > danger_threshold && cost_grid(y, x) != std::numeric_limits<double>::infinity()) 
							dangers.emplace_back(x, y);
					}
				}
				m_danger_tiles_is_cached = true;
			}
			
			if (!dangers.empty())
			{
				double closest_danger_distance = std::numeric_limits<double>::infinity();
				for (const auto& danger : dangers)
				{
					// std::cout << "Danger at: " << danger.first << " " << danger.second << std::endl;
					closest_danger_distance = std::min(
						(std::pow(danger.first - start.x, 2) + std::pow(danger.second - start.y, 2)),
						closest_danger_distance);
				}
				if (closest_danger_distance >= (danger_distance * danger_distance))
					return goal;
			}
			else return goal;
		}

		auto heightMap = ::sc2::HeightMap(m_bot.Observation()->GetGameInfo());

		// sensed danger and danger is within distance, perform custom pathfinding.
		auto path = AStarPathFind(start, goal, cost_grid, smoothing, sensitivity);

		return (path.size() > 1) ? path[1] : goal;
	}
}

