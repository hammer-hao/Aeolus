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
		case (constants::ManagerRequestType::FIND_CLOSEST_GROUND_SAFE_SPOT):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D, double>>(args);
			::sc2::Point2D position = std::get<0>(params);
			double radius = std::get<1>(params);
			return _getClosestSafeSpot(position, radius);
		}
		case (constants::ManagerRequestType::IS_GROUND_POSITION_SAFE):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D>>(args);
			::sc2::Point2D position = std::get<0>(params);
			return _isGroundPositionSafe(position);
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
			auto params = std::any_cast<std::tuple <::sc2::Point2D, ::sc2::Point2D, bool, int, float, bool, int>>(args);
			::sc2::Point2D start = std::get<0>(params);
			::sc2::Point2D goal = std::get<1>(params);
			bool sense_danger = std::get<2>(params);
			int danger_distance = std::get<3>(params);
			float danger_threshold = std::get<4>(params);
			bool smoothing = std::get<5>(params);
			int sensitivity = std::get<6>(params);
			return AStarPathFindNext(start, goal, m_ground_grid, sense_danger, danger_distance, danger_threshold, smoothing, sensitivity);
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

		if (constants::WEIGHT_COSTS.find(unit->unit_type)
			!= constants::WEIGHT_COSTS.end())
		{
			//  std::cout << "PathManager: found existing unit prifile! " << std::endl;
			// if we pre-defined unit ground/air weight and range
			auto it = constants::WEIGHT_COSTS.find(unit->unit_type);
			double ground_cost = it->second.GroundCost;
			double ground_range = it->second.GroundRange;

			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer, ground_cost);
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED)
		{
			// A disruptor Nova
			double ground_cost = 1000;
			double ground_range = 8 + Config::range_buffer;
			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range, ground_cost);

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
		
		else if (ManagerMediator::getInstance().CanAttackGround(m_bot, unit))
		{
			// std::cout << "PathManager: found unit with ground attack " << std::endl;
			double ground_range = ManagerMediator::getInstance().GroundRange(m_bot, unit);
			// std::cout << "PathManager: unit range acquired " << std::endl;
			double ground_dps = ManagerMediator::getInstance().GroundDPS(m_bot, unit);
			// std::cout << "PathManager: unit ground dps acquired " << std::endl;
			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer, ground_dps);

			if (ground_range < 2)
			{
				// melee units
			}
			else
			{
				// non-melee units
				// handle ground attack here

				if (ManagerMediator::getInstance().CanAttackAir(m_bot, unit))
				{
					// handle air attack
				}
			}
		}
		else if (ManagerMediator::getInstance().CanAttackAir(m_bot, unit))
		{
			// units with air attack only (no attack vs ground)
			// TODO: if unit is flying, get ground to air grid and air to air grid 
		}
	}

	::sc2::Point2D PathManager::_getClosestSafeSpot(::sc2::Point2D position, const double& radius)
	{
		return m_ground_grid.FindClosestSafeSpot(position, radius);
	}

	bool PathManager::_isGroundPositionSafe(::sc2::Point2D position)
	{
		return m_ground_grid.IsPositionSafe(position);
	}

	void PathManager::_reset_grids()
	{
		m_ground_grid.Reset();
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
		const Grid& grid, bool sense_danger, int danger_distance,
		float danger_threshold, bool smoothing, int sensitivity)
	{
		auto cost_grid = m_ground_grid.GetGrid();

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
						if (cost_grid(y, x) < danger_threshold) dangers.emplace_back(x, y);
					}
				}
				m_danger_tiles_is_cached = true;
			}
			
			if (!dangers.empty())
			{
				double closest_danger_distance = std::numeric_limits<double>::infinity();
				for (const auto& danger : dangers)
				{
					closest_danger_distance = std::min(
						(std::pow(danger.first, 2) + std::pow(danger.second, 2)),
						closest_danger_distance);
				}
				if (closest_danger_distance >= (danger_distance * danger_distance))
					return goal;
			}
			else return goal;
		}

		// sensed danger and danger is within distance, perform custom pathfinding.
		auto path = AStarPathFind(start, goal, cost_grid, smoothing, sensitivity);

		if (path.empty()) return goal;
		else return path.front();
	}
}

