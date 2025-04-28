#include "placement_manager.h"
#include "manager_mediator.h"
#include "../pathing/grid.h"
#include "../Aeolus.h"
#include "../utils/position_utils.h"
#include "../utils/Astar.hpp"
#include <chrono>
#include <sc2lib/sc2_search.h>
#include <sc2api/sc2_map_info.h>
#include <Eigen/Dense>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <cmath>
#include <tuple>
#include <stdexcept>
#include <optional>

namespace Aeolus
{
	PlacementManager::PlacementManager(AeolusBot& aeolusbot) : m_bot(aeolusbot), 
		m_height_map(aeolusbot.Observation()->GetGameInfo()),
		m_placement_grid(aeolusbot.Observation()->GetGameInfo())
	{
		m_expansion_map.resize(16);
	}

	std::any PlacementManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_EXPANSION_LOCATIONS):
		{
			// Grid placement_grid = Grid(m_bot.Observation()->GetGameInfo().placement_grid);
			// const auto height_map = ::sc2::HeightMap(m_bot.Observation()->GetGameInfo());
			return  _findExansionLocations();
		}
		case (constants::ManagerRequestType::REQUEST_BUILDING_PLACEMENT):
		{
			auto params = std::any_cast<std::tuple<int, ::sc2::UNIT_TYPEID, bool, bool, bool, bool, float, bool, ::sc2::Point2D>>(args);
			int base_index = std::get<0>(params);
			::sc2::UNIT_TYPEID structure_id = std::get<1>(params);
			bool is_wall = std::get<2>(params);
			bool find_alternative = std::get<3>(params);
			bool reserve_placement = std::get<4>(params);
			bool within_power_field = std::get<5>(params);
			float pylon_build_progress = std::get<6>(params);
			bool build_close_to = std::get<7>(params);
			::sc2::Point2D close_to = std::get<8>(params);
			return _requestBuildingPlacement(base_index, structure_id, is_wall, find_alternative, reserve_placement,
				within_power_field, pylon_build_progress, build_close_to, close_to);
		}
		default: return 0;
		}
	}

	void PlacementManager::update(int iteration)
	{
		// generate debug outputs for placements
		#ifdef BUILD_WITH_RENDERER

		// const auto height_map = ::sc2::HeightMap(m_bot.Observation()->GetGameInfo());
		auto* debug = m_bot.Debug();

		for (auto& expansion : m_expansion_map)
		{
			for (const auto& building : expansion[BuildingTypes::BUILDING_2X2])
			{
				if (true)
				{
					float terrain_height = m_height_map.TerrainHeight(::sc2::Point2D(building.first.first, building.first.second));
					::sc2::Point3D min = ::sc2::Point3D(building.first.first - 1, building.first.second - 1, terrain_height);
					::sc2::Point3D max = ::sc2::Point3D(building.first.first + 1, building.first.second + 1, terrain_height + 0.25);
					std::stringstream pos;
					pos << building.first.first << " " << building.first.second;
					debug->DebugTextOut(pos.str(), ::sc2::Point3D(building.first.first, building.first.second, terrain_height), ::sc2::Colors::Blue);
					debug->DebugBoxOut(min, max, ::sc2::Colors::Green);
				}
			}
			for (const auto& building : expansion[BuildingTypes::BUILDING_3X3])
			{
				if (true)
				{
					float terrain_height = m_height_map.TerrainHeight(::sc2::Point2D(building.first.first, building.first.second));
					::sc2::Point3D min = ::sc2::Point3D(building.first.first - 1.5, building.first.second - 1.5, terrain_height);
					::sc2::Point3D max = ::sc2::Point3D(building.first.first + 1.5, building.first.second + 1.5, terrain_height + 0.25);
					std::stringstream pos;
					pos << building.first.first << " " << building.first.second;
					debug->DebugTextOut(pos.str(), ::sc2::Point3D(building.first.first, building.first.second, terrain_height), ::sc2::Colors::Blue);
					debug->DebugBoxOut(min, max, ::sc2::Colors::Blue);
				}
			}
		}

		debug->SendDebug();
		#endif
	}

	void PlacementManager::Initialize()
	{
		auto start = std::chrono::high_resolution_clock::now();

		//solve placement logic

		Grid placement_grid = Grid(m_bot.Observation()->GetGameInfo().placement_grid);
		Grid pathing_grid = Grid(m_bot.Observation()->GetGameInfo().pathing_grid);
		// const auto height_map = ::sc2::HeightMap(m_bot.Observation()->GetGameInfo());

		auto placementGridNew = ::sc2::PlacementGrid(m_bot.Observation()->GetGameInfo());

		m_occupied_points.resize(placement_grid.GetWidth(), placement_grid.GetHeight()); // (occupied points is x, y)
		m_occupied_points.setZero();

		// set the status of "unbuildable plates" to occupied
		::sc2::Units allDestructibles = ManagerMediator::getInstance().GetAllDestructables(m_bot);
		for (const auto& destructible : allDestructibles)
		{
			if (constants::DESTRUCTABLE2X2_IDS.find(destructible->unit_type) != constants::DESTRUCTABLE2X2_IDS.end())
			{
				int xStart = static_cast<int>(destructible->pos.x - 1);
				int yStart = static_cast<int>(destructible->pos.y - 1);

				m_occupied_points.block<2, 2>(xStart, yStart).setConstant(1);
			}
		}


		std::vector<::sc2::Point2D> expansion_locations = _findExansionLocations();

		// get all ramp clusters first
		auto rampClusters = GetRampClusters(pathing_grid, placement_grid);

		_calculateProtossNaturalWall(expansion_locations, rampClusters);

		for (int i = 0; i < expansion_locations.size(); ++i)
		{
			// --- Add the town hall (5x5) at the center of the expansion ---
			// Record the town hall placement in the expansion map.
			_addPlacementPosition(BuildingTypes::BUILDING_5X5, i, expansion_locations[i], true, false, 0, false, 0.0, false, false);

			// avoid building within 9 distance of expansion location
			int start_x = static_cast<int>(std::round(expansion_locations[i].x - 4.5f));
			int start_y = static_cast<int>(std::round(expansion_locations[i].y - 4.5f));
			m_occupied_points.block<9, 9>(start_x, start_y).setConstant(1);

			// max distance for convolution fill
			int max_dist = 16;
			::sc2::Point2D wall_pylon;
			::sc2::Point2D ramp_position;

			if (expansion_locations[i] == ::sc2::Point2D(m_bot.Observation()->GetStartLocation()))
			{
				auto wall_positions = _calculateProtossRampPylonPos(::sc2::Point2D(m_bot.Observation()->GetStartLocation()), pathing_grid, placement_grid, rampClusters);
				wall_pylon = wall_positions[0];
				ramp_position = wall_positions[1];
				max_dist = 22;
			}

			auto area_points = ManagerMediator::getInstance().GetFloodFillArea(m_bot, expansion_locations[i], max_dist);

			std::cout << "BFS Flood fill: Found " << area_points.size() << " points. " << std::endl;

			/*
			float height = height_map.TerrainHeight(area_points[0]);
			for (const auto& point : area_points)
			{
				debug->DebugBoxOut(::sc2::Point3D(point.x, point.y, height), ::sc2::Point3D(point.x + 1, point.y + 1, height + 0.25), ::sc2::Colors::Gray);
			}

			debug->SendDebug();

			*/

			auto bounding_box = utils::GetBoundingBox(area_points);

			// find pylon locations first
			Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> kernel(2, 2);
			kernel.setOnes();
			auto pylon_locations = _findBuildingLocations(kernel, bounding_box, 7, 7, placementGridNew, pathing_grid, m_occupied_points, 2, 2);

			std::cout << "found " << pylon_locations.size() << " pylon locations." << std::endl;

			for (const auto& location : pylon_locations)
			{
				if (i == 0 && ::sc2::DistanceSquared2D(wall_pylon, location) < 56.0f) continue;
				if (m_height_map.TerrainHeight(location) == m_height_map.TerrainHeight(expansion_locations[i]))
				{
					_addPlacementPosition(BuildingTypes::BUILDING_2X2, i, location, true, false, 0, false, 0.0, true, false);
					int x_begin = static_cast<int>(std::round(location.x - 1));
					int y_begin = static_cast<int>(std::round(location.y - 1));
					m_occupied_points.block<2, 2>(x_begin, y_begin).setConstant(1);
				}
			}

			// find 3x3 positions for gateways, cybercore, robo, etc
			kernel = Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>(3, 3);
			kernel.setOnes();
			auto threebythree_locations = _findBuildingLocations(kernel, bounding_box, 3, 3, placementGridNew, pathing_grid, m_occupied_points, 3, 3);
			size_t num_threebythree = threebythree_locations.size();

			std::cout << "found " << num_threebythree << " three by three locations." << std::endl;

			for (int j = 0; j < num_threebythree; ++j)
			{
				// drop some placements to avoid walling in
				if (num_threebythree > 6 && j % 4 == 0) continue;
				if (m_height_map.TerrainHeight(threebythree_locations[j]) == m_height_map.TerrainHeight(expansion_locations[i]))
				{
					_addPlacementPosition(BuildingTypes::BUILDING_3X3, i, threebythree_locations[j]);
					int x_begin = static_cast<int>(std::round(threebythree_locations[j].x - 1.5f));
					int y_begin = static_cast<int>(std::round(threebythree_locations[j].y - 1.5f));
					m_occupied_points.block<3, 3>(x_begin, y_begin).setConstant(1);
				}
			}

			// lastly, find 2x2 positions for additional pylons, cannons, batteries, dark shrines, etc
			kernel = Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>(2, 2);
			kernel.setOnes();
			auto twobytwo_locations = _findBuildingLocations(kernel, bounding_box, 2, 2, placementGridNew, pathing_grid, m_occupied_points, 2, 2);
			size_t num_twobytwo = twobytwo_locations.size();

			std::cout << "found " << num_twobytwo << " two by two locations." << std::endl;

			for (int j = 0; j < num_twobytwo; ++j)
			{
				// don't add any near the top ramp
				if (i == 0 && ::sc2::DistanceSquared2D(twobytwo_locations[j], ramp_position) < 30.0f) continue;

				// drop some placements to avoid walling in
				if (num_twobytwo > 6 && j % 5 == 0) continue;

				if (m_height_map.TerrainHeight(twobytwo_locations[j]) == m_height_map.TerrainHeight(expansion_locations[i]))
				{
					_addPlacementPosition(BuildingTypes::BUILDING_2X2, i, twobytwo_locations[j]);
					int x_begin = static_cast<int>(std::round(twobytwo_locations[j].x - 1));
					int y_begin = static_cast<int>(std::round(twobytwo_locations[j].y - 1));
					m_occupied_points.block<2, 2>(x_begin, y_begin).setConstant(1);
				}
			}

			// find optimal pylon for base
			std::optional<std::pair<float, float>> best_pylon_pos = std::nullopt;
			int most_three_by_threes = 0;
			for (const auto& two_by_two : m_expansion_map[i][BuildingTypes::BUILDING_2X2])
			{
				if (two_by_two.second.production_pylon)
				{
					// is a production pylon
					// check the number of three by threes in its range
					int this_three_by_threes = 0;
					for (const auto& three_by_three : m_expansion_map[i][BuildingTypes::BUILDING_3X3])
					{
						if (::sc2::DistanceSquared2D(::sc2::Point2D(two_by_two.first.first, two_by_two.first.second),
							::sc2::Point2D(three_by_three.first.first, three_by_three.first.second)) < 42.25)
							this_three_by_threes++;
					}
					if (this_three_by_threes > most_three_by_threes)
					{
						most_three_by_threes = this_three_by_threes;
						best_pylon_pos = two_by_two.first;
					}
				}
			}
			if (best_pylon_pos.has_value()) 
				m_expansion_map[i][BuildingTypes::BUILDING_2X2][best_pylon_pos.value()].optimal_pylon = true;
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << "Completed placement calculation in " << duration.count() << " ms." << std::endl;
	}

	std::vector<std::vector<std::pair<int, int>>>
		PlacementManager::GetRampClusters(const Grid& pathing_grid,
			const Grid& placement_grid,
			std::size_t min_cluster_size)
	{
		std::vector<std::vector<std::pair<int, int>>> ramp_clusters;
		std::set<std::pair<int, int>> unvisited;

		Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> pathing_matrix = pathing_grid.GetGrid();
		Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> placement_matrix = placement_grid.GetGrid();

		// 1) Fill 'unvisited' with pathable && not placable points
		for (int y = 0; y < pathing_grid.GetHeight(); ++y) {
			for (int x = 0; x < pathing_grid.GetWidth(); ++x) {
				if (pathing_matrix(y, x) == 1 && placement_matrix(y, x) == 0)
				{
					unvisited.insert({ x, y });
				}
			}
		}

		while (!unvisited.empty())
		{
			// start a new cluster
			std::vector<std::pair<int, int>> current_cluster;
			std::queue<std::pair<int, int>> to_visit;

			// Grab any unvisited tile as BFS seed
			auto seed_it = unvisited.begin();
			std::pair<int, int> seed = *seed_it;
			unvisited.erase(seed);
			to_visit.push(seed);

			while (!to_visit.empty())
			{
				std::pair<int, int> base = to_visit.front();
				to_visit.pop();
				current_cluster.push_back(base);

				for (int dy = -1; dy <= 1; ++dy)
				{
					for (int dx = -1; dx <= 1; ++dx)
					{
						if (dx == 0 && dy == 0) continue;
						std::pair<int, int> neighbor = { base.first + dx, base.second + dy };

						auto it = unvisited.find(neighbor);
						if (it != unvisited.end())
						{
							unvisited.erase(it);
							to_visit.push(neighbor);
						}
					}
				}
			}
			if (current_cluster.size() >= 8) ramp_clusters.push_back(current_cluster);
		}

		std::cout << "BFS: found " << ramp_clusters.size() << " ramp clusters." << std::endl;
		return ramp_clusters;
	}

	std::vector<::sc2::Point2D> PlacementManager::_calculateProtossRampPylonPos(::sc2::Point2D main_location,
		const Grid& pathing_grid, const Grid& placement_grid, const std::vector<std::vector<std::pair<int, int>>>& ramp_clusters)
	{
		float x_offset = 0.5, y_offset = 0.5;

		// find the ramp closes to the main base
		int best_index = -1;
		float best_distance_sq = std::numeric_limits<float>::max();

		for (int i = 0; i < ramp_clusters.size(); ++i)
		{
			std::pair<int, int> avg = { 0, 0 };
			float highest = std::numeric_limits<float>::min();
			float lowest = std::numeric_limits<float>::max();
			for (auto& p : ramp_clusters[i])
			{
				avg.first += p.first;
				avg.second += p.second;
				float terrain_height = m_height_map.TerrainHeight(::sc2::Point2D(p.first, p.second));
				if (terrain_height > highest) highest = terrain_height;
				if (terrain_height < lowest) lowest = terrain_height;
			}
			avg.first /= ramp_clusters[i].size();
			avg.second /= ramp_clusters[i].size();

			float distance_sq = std::pow(avg.first - main_location.x, 2) + std::pow(avg.second - main_location.y, 2);
			if (distance_sq < best_distance_sq && highest - lowest > 1)
			{
				best_distance_sq = distance_sq;
				best_index = i;
			}
		}

		const std::vector<std::pair<int, int>>& main_ramp_cluster = ramp_clusters[best_index];

		auto* debug = m_bot.Debug();

		for (const auto& point : main_ramp_cluster)
		{
			float z = m_height_map.TerrainHeight(::sc2::Point2D(point.first, point.second));
			// debug->DebugSphereOut(::sc2::Point3D(point.first, point.second, z), 0.5, ::sc2::Colors::Green);
		}

		// 2) Identify the ramp's "upper" points
		//    (the ones matching the maximum terrain height within the ramp).
		float maxHeight = -9999.0f;
		std::vector<std::pair<int, int>> rampUpperPoints;
		for (auto& pt : main_ramp_cluster)
		{
			float terrain_height = m_height_map.TerrainHeight(::sc2::Point2D(pt.first, pt.second));
			std::cout << "Terrain height:" << terrain_height << std::endl;
			if (terrain_height > maxHeight) {
				maxHeight = terrain_height;
				rampUpperPoints.clear();
				rampUpperPoints.push_back(pt);
			}
			else if (terrain_height == maxHeight) {
				rampUpperPoints.push_back(pt);
			}
		}

		std::cout << "Found " << rampUpperPoints.size() << " Ramp upper points." << std::endl;

		float UpperxSum = 0, UpperySum = 0;
		for (const auto& point : rampUpperPoints)
		{
			float z = m_height_map.TerrainHeight(::sc2::Point2D(point.first, point.second));
			// debug->DebugSphereOut(::sc2::Point3D(point.first, point.second, z), 0.5, ::sc2::Colors::Red);
			UpperxSum += point.first;
			UpperySum += point.second;
		}
		::sc2::Point2D upper_middle = { UpperxSum / rampUpperPoints.size(), UpperySum / rampUpperPoints.size() };
		// debug->DebugSphereOut(::sc2::Point3D(upper_middle.x, upper_middle.y, height_map.TerrainHeight(upper_middle)), 0.5, ::sc2::Colors::Purple);

		// 3) Compute the ramp's bottom center by averaging the lowest points.
		std::vector<std::pair<int, int>> rampLowerPoints;
		float minHeight = 9999.0f;
		for (auto& pt : main_ramp_cluster) {
			float h = m_height_map.TerrainHeight(::sc2::Point2D(pt.first, pt.second));
			if (h < minHeight) {
				minHeight = h;
				rampLowerPoints.clear();
				rampLowerPoints.push_back(pt);
			}
			else if (h == minHeight) {
				rampLowerPoints.push_back(pt);
			}
		}

		float lowerxSum = 0, lowerySum = 0;
		for (const auto& point : rampLowerPoints)
		{
			float z = m_height_map.TerrainHeight(::sc2::Point2D(point.first, point.second));
			// debug->DebugSphereOut(::sc2::Point3D(point.first, point.second, z), 0.5, ::sc2::Colors::Blue);
			lowerxSum += point.first;
			lowerySum += point.second;
		}
		::sc2::Point2D lower_middle = { lowerxSum / rampLowerPoints.size(), lowerySum / rampLowerPoints.size() };
		// debug->DebugSphereOut(::sc2::Point3D(lower_middle.x, lower_middle.y, height_map.TerrainHeight(lower_middle)), 0.5, ::sc2::Colors::Purple);

		std::cout << "Found " << rampLowerPoints.size() << " Ramp lower points." << std::endl;

		if (rampUpperPoints.size() >= 2)
		{
			::sc2::Point2D p1 = { rampUpperPoints[0].first + x_offset, rampUpperPoints[0].second + y_offset };
			::sc2::Point2D p2 = { rampUpperPoints[1].first + x_offset, rampUpperPoints[1].second + y_offset };

			float halfdist = ::sc2::Distance2D(p1, p2) / 2.0f;
			::sc2::Point2D center = utils::GetPositionTowards(p1, p2, halfdist);
			auto intersects = utils::circleIntersection(p1, p2, std::sqrt(2.5));
			auto bestIt = std::max_element(
				intersects.begin(),
				intersects.end(),
				[&](const ::sc2::Point2D a, const ::sc2::Point2D b) {
					return ::sc2::Distance2D(a, ::sc2::Point2D(rampLowerPoints[0].first, rampLowerPoints[0].second))
						< ::sc2::Distance2D(b, ::sc2::Point2D(rampLowerPoints[0].first, rampLowerPoints[0].second));
				}
			);
			auto middle_depot = *bestIt;

			std::vector<::sc2::Point2D> depot_intersects = utils::circleIntersection(middle_depot, center, std::sqrt(5));

			::sc2::Point2D ramp_direction = (upper_middle - lower_middle) / ::sc2::Distance2D(upper_middle, lower_middle);
			::sc2::Point2D pylon_position_float = upper_middle + ramp_direction * 6.0f;
			::sc2::Point2D pylon_position = { static_cast<float>(std::round(pylon_position_float.x)), static_cast<float>(std::round(pylon_position_float.y)) };

			//::sc2::Point2D building1_position_float = upper_middle + ramp_direction * 2.0f;
			::sc2::Point2D building1_position_float = depot_intersects[0] + ramp_direction;
			::sc2::Point2D building1_position = { static_cast<float>(std::round(building1_position_float.x + 0.5f)) - 0.5f,
				static_cast<float>(std::round(building1_position_float.y + 0.5f)) - 0.5f };

			::sc2::Point2D perpendicular_direction = { ramp_direction.y, -ramp_direction.x };
			// ::sc2::Point2D building2_position_float = upper_middle + ramp_direction * 1.5f + perpendicular_direction * 3.5f;
			::sc2::Point2D building2_position_float = middle_depot + ramp_direction + (middle_depot - building1_position) / 1.5f;
			::sc2::Point2D building2_position = { static_cast<float>(std::round(building2_position_float.x + 0.5f)) - 0.5f,
				static_cast<float>(std::round(building2_position_float.y + 0.5f)) - 0.5f };

			// debug->DebugSphereOut(::sc2::Point3D(building1_position_float.x, building1_position_float.y, height_map.TerrainHeight(building1_position_float)), 0.5, ::sc2::Colors::Purple);

			// debug->SendDebug();

			// add the calculated positions to our placement map
			_addPlacementPosition(BuildingTypes::BUILDING_2X2, 0, pylon_position, true, true, 0, false, 0.0, true, true);
			_addPlacementPosition(BuildingTypes::BUILDING_3X3, 0, building1_position, true, true);
			_addPlacementPosition(BuildingTypes::BUILDING_3X3, 0, building2_position, true, true);

			int pylon_x = static_cast<int>(pylon_position.x - 1.0);
			int pylon_y = static_cast<int>(pylon_position.y - 1.0);
			int building1_x = static_cast<int>(building1_position.x - 1.5);
			int building1_y = static_cast<int>(building1_position.y - 1.5);
			int building2_x = static_cast<int>(building2_position.x - 1.5);
			int building2_y = static_cast<int>(building2_position.y - 1.5);

			m_occupied_points.block<2, 2>(pylon_x, pylon_y).setConstant(1);
			m_occupied_points.block<3, 3>(building1_x, building1_y).setConstant(1);
			m_occupied_points.block<3, 3>(building2_x, building2_y).setConstant(1);

			return { pylon_position, upper_middle };
		}
	}

	std::vector<std::pair<double, double>> PlacementManager::makeSampleAxis(int m)
	{
		std::vector<std::pair<double, double>> out;
		out.reserve(m);
		for (int i = 0; i < m; ++i) {
			double angle = (static_cast<double>(i) / static_cast<double>(m)) * 3.14159265358979323846;
			double c = std::cos(angle);
			double s = std::sin(angle);
			out.emplace_back(c, s);
		}
		return out;
	}

	std::pair<std::pair<int, int>, std::pair<int, int>> PlacementManager::findChoke(
		std::vector<::sc2::Point2D> refs, 
		const ::sc2::PlacementGrid& placementGrid,
		const ::sc2::HeightMap& heightMap,
		int radius,
		int targetWidth)
	{
		auto pathingGrid = ::sc2::PathingGrid(m_bot.Observation()->GetGameInfo());

		auto axes = makeSampleAxis(16);

		std::pair<double, double> optimalAxes = { 0.0, 0.0 };
		::sc2::Point2D bestRef = { 0.0f, 0.0f };
		double  globalBestScore = std::numeric_limits<double>::infinity();

		for (const auto& ref : refs)
		{
			std::cout << "ref: " << ref.x << " " << ref.y << std::endl;
			double localBestScore = std::numeric_limits<double>::infinity();
			std::pair<double, double> bestAxis = { 0.0, 0.0 };
			
			for (const auto& axis : axes)
			{
				int pos = 0;
				int neg = 0;
				bool valid = true;
				int refHeight = heightMap.TerrainHeight({static_cast<int>(ref.x), static_cast<int>(ref.y)});
				for (int i = 0; i < radius; ++i)
				{
					int row = static_cast<int>(ref.x + i * axis.first);
					int col = static_cast<int>(ref.y + i * axis.second);

					if (std::abs(heightMap.TerrainHeight({ row, col }) - refHeight) > 0.001 &&
						std::abs(heightMap.TerrainHeight({ row, col }) - refHeight) < 1.5)
					{
						valid = false;
						break;
					}

					if (placementGrid.IsPlacable({ row, col }) == false)
					{
						if (pathingGrid.IsPathable({ row, col })) valid = false;
						break;
					}
					pos++;
				}
				if (!valid) continue;

				for (int i = 0; i < radius; ++i)
				{
					int row = static_cast<int>(ref.x - i * axis.first);
					int col = static_cast<int>(ref.y - i * axis.second);

					if (std::abs(heightMap.TerrainHeight({ row, col }) - refHeight) > 0.001 &&
						std::abs(heightMap.TerrainHeight({ row, col }) - refHeight) < 1.5)
					{
						valid = false;
						break;
					}

					if (placementGrid.IsPlacable({ row, col }) == false)
					{
						if (pathingGrid.IsPathable({ row, col })) valid = false;
						break;
					}
					neg++;
				}
				if (!valid) continue;

				int width = pos + neg + 1;

				double score = std::abs(width - targetWidth);
				std::cout << "width: " << width << std::endl;
				if (score < localBestScore)
				{
					localBestScore = score;
					bestAxis = axis;
				}
			}

			if (localBestScore < globalBestScore)
			{
				globalBestScore = localBestScore;
				bestRef = ref;
				optimalAxes = bestAxis;
			}
		}

		std::pair<int, int> end1 = { 0, 0 };
		std::pair<int, int> end2 = { 0, 0 };

		float bestRefHeight = heightMap.TerrainHeight({ static_cast<int>(bestRef.x), static_cast<int>(bestRef.y) });

		for (int i = 0; i < radius; ++i)
		{
			int x = static_cast<int>(bestRef.x + i * optimalAxes.first);
			int y = static_cast<int>(bestRef.y + i * optimalAxes.second);
#ifdef BUILD_WITH_RENDERER
			float z = m_height_map.TerrainHeight({ x, y });
			m_bot.Debug()->DebugBoxOut({x - 0.5f, y - 0.5f, z}, {x + 0.5f, y + 0.5f, z + 0.2f}, ::sc2::Colors::Yellow);
#endif
			if (!placementGrid.IsPlacable({ x, y }) ||
				(std::abs(heightMap.TerrainHeight({x, y}) - bestRefHeight) > 0.1f))
			{
				end1 = { x, y };
				break;
			}
		}
		for (int i = 0; i < radius; ++i)
		{
			int x = static_cast<int>(bestRef.x - i * optimalAxes.first);
			int y = static_cast<int>(bestRef.y - i * optimalAxes.second);
#ifdef BUILD_WITH_RENDERER
			float z = m_height_map.TerrainHeight({ x, y });
			m_bot.Debug()->DebugBoxOut({ x - 0.5f, y - 0.5f, z }, { x + 0.5f, y + 0.5f, z + 0.2f }, ::sc2::Colors::Yellow);
#endif
			if (!placementGrid.IsPlacable({ x, y }) ||
				(std::abs(heightMap.TerrainHeight({ x, y }) - bestRefHeight) > 0.1f))
			{
				end2 = { x, y };
				break;
			}
		}

		return { end1, end2 };
	}

	std::pair<std::pair<int, int>, std::pair<int, int>> PlacementManager::findChokeByRamp(
		const std::vector<std::vector<std::pair<int, int>>>& ramp_clusters,
		const ::sc2::HeightMap& heightMap,
		const ::sc2::PlacementGrid& placementGrid,
		const ::sc2::PathingGrid& pathingGrid,
		::sc2::Point2D naturalPos
	)
	{
		// find the ramp closes to the natural base
		int best_index = -1;
		float best_distance_sq = std::numeric_limits<float>::max();

		for (int i = 0; i < ramp_clusters.size(); ++i)
		{
			if (ramp_clusters[i].size() <= 16) continue;
			std::pair<int, int> avg = { 0, 0 };
			float highest = std::numeric_limits<float>::min();
			float lowest = std::numeric_limits<float>::max();
			for (auto& p : ramp_clusters[i])
			{
				avg.first += p.first;
				avg.second += p.second;
				float terrain_height = m_height_map.TerrainHeight(::sc2::Point2D(p.first, p.second));
				if (terrain_height > highest) highest = terrain_height;
				if (terrain_height < lowest) lowest = terrain_height;
			}
			avg.first /= ramp_clusters[i].size();
			avg.second /= ramp_clusters[i].size();

			float distance_sq = std::pow(avg.first - naturalPos.x, 2) + std::pow(avg.second - naturalPos.y, 2);
			if (distance_sq < best_distance_sq && highest - lowest > 1)
			{
				best_distance_sq = distance_sq;
				best_index = i;
			}
		}

		const std::vector<std::pair<int, int>>& natural_ramp_cluster = ramp_clusters[best_index];

		float highest = std::numeric_limits<float>::min();

		for (const auto& pt : natural_ramp_cluster)
		{
			float z = m_height_map.TerrainHeight({ pt.first, pt.second });
			
			if (z > highest) highest = z;
		}

		std::vector<std::pair<int, int>> topPoints;

		std::copy_if(natural_ramp_cluster.begin(), natural_ramp_cluster.end(), std::back_inserter(topPoints),
			[&](const std::pair<int, int>& pt)
			{
				return m_height_map.TerrainHeight({ pt.first, pt.second }) == highest;
			}
		);

		// find the two top points that are furthest apart
		if (topPoints.size() < 2) return { natural_ramp_cluster.front(), natural_ramp_cluster.back() }; // in case of not enough top points
		std::pair<int, int> pt1, pt2;
		double maxDist = -1.0;

		for (size_t i = 0; i < topPoints.size(); ++i)
		{
			for (size_t j = i + 1; j < topPoints.size(); ++j)
			{
				double dx = topPoints[i].first - topPoints[j].first;
				double dy = topPoints[i].second - topPoints[j].second;
				double dist = std::hypot(dx, dy);

				if (dist > maxDist)
				{
					maxDist = dist;
					pt1 = topPoints[i];
					pt2 = topPoints[j];
				}
			}
		}

		//std::cout << "pt1: " << pt1.first << " " << pt1.second << std::endl;
		//std::cout << "pt2: " << pt2.first << " " << pt2.second << std::endl;

		double dx = (pt2.first - pt1.first)/static_cast<double>(topPoints.size());
		double dy = (pt2.second - pt1.second)/static_cast<double>(topPoints.size());

		//std::cout << "dx: " << dx << std::endl;
		//std::cout << "dy: " << dy << std::endl;
		
		std::pair<int, int> end1 = { 0, 0 };
		std::pair<int, int> end2 = { 0, 0 };

		for (int i = 0; i < 15; ++i)
		{
			// keep in the direction of the line formed by the top ramp points
			// until we hit a tile that is not pathable and not placable
			// this tile will be the end
			int x = static_cast<int>(pt2.first + dx * i);
			int y = static_cast<int>(pt2.second + dy * i);
			if (!placementGrid.IsPlacable({ x, y }) && !pathingGrid.IsPathable({ x, y }))
			{
				end1 = { x, y };
				break;
			}
		}

		for (int i = 0; i < 15; ++i)
		{
			int x = static_cast<int>(pt1.first - dx * i);
			int y = static_cast<int>(pt1.second - dy * i);
			if (!placementGrid.IsPlacable({ x, y }) && !pathingGrid.IsPathable({ x, y }))
			{
				end2 = { x, y };
				break;
			}
		}


#ifdef BUILD_WITH_RENDERER

		auto* debug = m_bot.Debug();
		for (const auto& pt : {pt1, pt2})
		{
			float z = m_height_map.TerrainHeight({ pt.first, pt.second });
			float x = static_cast<float>(pt.first);
			float y = static_cast<float>(pt.second);

			debug->DebugBoxOut({ x, y, z }, { x + 1.0f, y + 1.0f, z + 0.2f }, ::sc2::Colors::Yellow);
		}

		for (const auto& pt : { end1, end2 })
		{
			float z = m_height_map.TerrainHeight({ pt.first, pt.second });
			float x = static_cast<float>(pt.first);
			float y = static_cast<float>(pt.second);

			debug->DebugBoxOut({ x, y, z }, { x + 1.0f, y + 1.0f, z + 0.2f }, ::sc2::Colors::Purple);
		}

		debug->SendDebug();
#endif
		std::cout << "solved natural ramp ends: " << end1.first << " " << end1.second << ", "
			<< end2.first << " " << end2.second << std::endl;
		return { end1, end2 };
	}

	std::pair<int, int> PlacementManager::findNatural3x3Placment(std::pair<int, int> startEnd, std::pair<int, int> targetEnd, 
		const ::sc2::PlacementGrid& placementGrid, std::pair<int, int> reference)
	{
		std::vector<std::pair<int, int>> offsets =
		{
			{-1, -2}, { 0, -2}, { 1, -2},
			{ 2, -1}, { 2,  0}, { 2,  1},
			{-1,  2}, { 0,  2}, { 1,  2},
			{-2, -1}, {-2,  0}, {-2,  1}
		};

		auto sqDist = [&](const std::pair<int, int>& off) {
			float dx = float(startEnd.first + off.first) - float(targetEnd.first);
			float dy = float(startEnd.second + off.second) - float(targetEnd.second);
			return dx * dx + dy * dy;
			};
		
		std::sort(offsets.begin(), offsets.end(), [&](const auto& a, const auto& b) {
			return sqDist(a) < sqDist(b);
			}
		);

		for (const auto& offset : offsets)
		{
			bool valid = true;
			for (int i = -1; i < 2; ++i)
			{
				for (int j = -1; j < 2; ++j)
				{
					int x = startEnd.first + offset.first + i;
					int y = startEnd.second + offset.second + j;
					if (!placementGrid.IsPlacable({ x, y }) || m_occupied_points(x, y) == 1)
					{
						valid = false;
						break;
					}
					/*
					if (reference != std::pair<int, int>(0, 0))
					{
						if ((x == reference.first && std::abs(y - reference.second) == 6) ||
							(y == reference.second && std::abs(x - reference.first) == 6))
						{
							std::cout << "invalid formation!" << std::endl;
							valid = false;
							break;
						}
					}
					*/
				}
				if (!valid) break;
			}
			if (!valid) continue;

			return { startEnd.first + offset.first, startEnd.second + offset.second };
		}
		return { 0, 0 };
	}

	std::pair<std::pair<int, int>, std::pair<int, int>> PlacementManager::findNaturalFinal3x3Placement(std::pair<int, int> pos1,
		std::pair<int, int> pos2, const ::sc2::PlacementGrid& placementGrid, ::sc2::Point2D naturalPos)
	{
		const std::vector<std::pair<int, int>> offsets =
		{
			{-2, -3}, {-1, -3}, { 0, -3}, { 1, -3}, { 2, -3},
			{ 3, -2}, { 3, -1}, { 3,  0}, { 3,  1}, { 3,  2},
			{-2,  3}, {-1,  3}, { 0,  3}, { 1,  3}, { 2,  3},
			{-3, -2}, {-3, -1}, {-3,  0}, {-3,  1}, {-3,  2}
		};

		std::vector<std::pair<int, int>> validOffsets;
		std::copy_if(offsets.begin(), offsets.end(), std::back_inserter(validOffsets),
			[&](std::pair<int, int> offset)
			{
				if (std::abs(pos1.first + offset.first - pos2.first) == 4)
				{
					if (std::abs(pos1.second + offset.second - pos2.second) <= 2)
					{
						return true;
					}
				}
				if (std::abs(pos1.second + offset.second - pos2.second) == 4)
				{
					if (std::abs(pos1.first + offset.first - pos2.first) <= 2)
					{
						return true;
					}
				}
				return false;
			}
		);

		auto sqDist = [&](const std::pair<int, int>& off) {
			float dx = float(pos1.first + off.first) - float(naturalPos.x);
			float dy = float(pos1.second + off.second) - float(naturalPos.y);
			return dx * dx + dy * dy;
			};

		std::sort(validOffsets.begin(), validOffsets.end(), [&](const auto& a, const auto& b) {
			return sqDist(a) > sqDist(b);
			}
		);

		// sorted possible placement candidates, now find the first valid one
		for (const auto& offset : validOffsets)
		{
			bool valid = true;
			for (int i = -1; i < 2; ++i)
			{
				for (int j = -1; j < 2; ++j)
				{
					int x = pos1.first + offset.first + i;
					int y = pos1.second + offset.second + j;
					if (!placementGrid.IsPlacable({ x, y }) || m_occupied_points(x, y) == 1)
					{
						valid = false;
						break;
					}
				}
				if (!valid) break;
			}
			if (!valid) continue;

			// found valid candidate
			std::pair<int, int> pos3 = { pos1.first + offset.first, pos1.second + offset.second };

			std::pair<int, int> unitPos = { 
				(pos1.first + offset.first - pos2.first) / 2,
				(pos1.second + offset.second - pos2.second) / 2 };

			return { pos3, unitPos };
		}
		return { {0,0}, {0, 0} };
	}

	std::set<std::pair<int, int>> PlacementManager::pointsWithinDistance(std::pair<int, int> center, float maxDistance)
	{
		std::set<std::pair<int, int>> result;
		for (int dx = std::floor(-maxDistance); dx <= std::ceil(maxDistance); dx++)
		{
			for (int dy = std::floor(-maxDistance); dy <= std::ceil(maxDistance); dy++)
			{
				if (std::hypot(dx - 0.5, dy - 0.5) <= maxDistance)
				{
					result.insert({ center.first + dx, center.second + dy });
				}
			}
		}
		return result;
	}

	std::pair<int, int> PlacementManager::findNaturalPylonPlacement(std::vector<std::pair<int, int>> threeByThrees, 
		const ::sc2::PlacementGrid& placementGrid, ::sc2::Point2D naturalPos)
	{
		if (threeByThrees.empty()) 
		{
			std::cout << "Cannot find natural pylon placement: no wall position given!" << std::endl;
			return { 0, 0 };
		}

		constexpr float maxDistance = 6.5f;

		std::vector<std::set<std::pair<int, int>>> allPos;

		for (const auto& pos : threeByThrees)
		{
			allPos.push_back(pointsWithinDistance(pos, maxDistance));
		}

		// Start with a copy of the first set
		std::set<std::pair<int, int>> intersection = allPos.front();

		for (size_t i = 1; i < allPos.size(); ++i)
		{
			std::set<std::pair<int, int>> temp;
			std::set_intersection(
				intersection.begin(), intersection.end(),
				allPos[i].begin(), allPos[i].end(),
				std::inserter(temp, temp.begin())
			);

			intersection = std::move(temp);

			if (intersection.empty()) break;
		}

		if (intersection.empty())
		{
			std::cout << "Cannot find a valid wall pylon position for the natural! " << std::endl;
		}

		std::vector<std::pair<int, int>> validPylons;

		std::copy_if(intersection.begin(), intersection.end(), std::back_inserter(validPylons),
			[&](const std::pair<int, int>& pos)
			{
				for (int dx = -1; dx < 1; ++dx)
				{
					for (int dy = -1; dy < 1; ++dy)
					{
						if (!placementGrid.IsPlacable({ pos.first + dx, pos.second + dy })) return false;
						if (m_occupied_points(pos.first + dx, pos.second + dy) == 1) return false;
					}
				}
				return true;
			}
		);

		auto sqDist = [&](const std::pair<int, int>& pylonPos) {
			float dx = float(pylonPos.first) - float(naturalPos.x);
			float dy = float(pylonPos.second) - float(naturalPos.y);
			return dx * dx + dy * dy;
			};

		std::sort(validPylons.begin(), validPylons.end(), [&](const std::pair<int, int>& a, const std::pair<int, int>& b)
			{
				return sqDist(a) < sqDist(b);
			}
		);

		if (!validPylons.empty())
		{
			// use the closest pylon to nexus as our wall pylon
			return validPylons.front();
		}

		std::cout << "Cannot find a valid pylon to power all natural wall 3x3s!" << std::endl;
		return { 0, 0 };
	}

	void PlacementManager::_calculateProtossNaturalWall(std::vector<::sc2::Point2D> expansion_locations,
		const std::vector<std::vector<std::pair<int, int>>>& ramp_clusters)
	{
		if (expansion_locations.size() < 2) return; // not a valid map for this calculation

		::sc2::Point2D naturalPos = expansion_locations[1];
		::sc2::Point2D enemyBase = expansion_locations.back();

		auto placementGrid = ::sc2::PlacementGrid(m_bot.Observation()->GetGameInfo());
		auto pathingGrid = ::sc2::PathingGrid(m_bot.Observation()->GetGameInfo());

		float naturalHeight = m_height_map.TerrainHeight({ static_cast<int>(naturalPos.x), static_cast<int>(naturalPos.y) });

		std::vector<::sc2::Point2D> astarPath = AStarPathFind(naturalPos, enemyBase,
			ManagerMediator::getInstance().GetAstarGrid(m_bot).GetGrid());

		// for (const auto& pathPoint : astarPath)
		// {
		//	// std::cout << "pathpoint: " << pathPoint.x << " " << pathPoint.y << std::endl;
		// }

		std::vector<::sc2::Point2D> refs;

		std::copy_if(astarPath.begin(), astarPath.end(), std::back_inserter(refs),
			[&](::sc2::Point2D point) {
				return (sc2::DistanceSquared2D(point, expansion_locations[1]) < 256 && sc2::DistanceSquared2D(point, expansion_locations[1]) > 100);
			});

		bool rampWall = false;

		for (const auto& ref : refs)
		{
			if (naturalHeight - m_height_map.TerrainHeight({ static_cast<int>(ref.x), static_cast<int>(ref.y) }) >= 0.1f)
			{
				rampWall = true;
				break;
			}
		}

		std::pair<int, int> end1, end2;

		if (rampWall)
		{
			auto twoEnds = findChokeByRamp(ramp_clusters, m_height_map, placementGrid, pathingGrid, expansion_locations[1]);

			end1 = twoEnds.first;
			end2 = twoEnds.second;
		}
		else
		{
			auto twoEnds = findChoke(refs, placementGrid, m_height_map);

			end1 = twoEnds.first;
			end2 = twoEnds.second;
		}

		/*

#ifdef BUILD_WITH_RENDERER

		auto* debug = m_bot.Debug();

		for (const auto& pathPoint : astarPath)
		{
			int x = static_cast<int>(pathPoint.x);
			int y = static_cast<int>(pathPoint.y);
			float z = m_height_map.TerrainHeight({ x, y });
			debug->DebugBoxOut({ pathPoint.x - 0.5f, pathPoint.y - 0.5f, z }, { pathPoint.x + 0.5f, pathPoint.y + 0.5f, z + 0.2f }, ::sc2::Colors::Red);
		}

		for (const auto& pathPoint : refs)
		{
			int x = static_cast<int>(pathPoint.x);
			int y = static_cast<int>(pathPoint.y);
			float z = m_height_map.TerrainHeight({ x, y });
			debug->DebugBoxOut({ pathPoint.x - 0.5f, pathPoint.y - 0.5f, z }, { pathPoint.x + 0.5f, pathPoint.y + 0.5f, z + 0.2f }, ::sc2::Colors::Green);
		}

		if (end1 != std::pair<int, int>(0, 0))
		{
			int x = end1.first;
			int y = end1.second;
			float z = m_height_map.TerrainHeight({ x, y });
			debug->DebugBoxOut({ x - 0.5f, y - 0.5f, z }, { x + 0.5f, y + 0.5f, z + 0.2f }, ::sc2::Colors::Purple);
		}

		if (end2 != std::pair<int, int>(0, 0))
		{
			int x = end2.first;
			int y = end2.second;
			float z = m_height_map.TerrainHeight({ x, y });
			debug->DebugBoxOut({ x - 0.5f, y - 0.5f, z }, { x + 0.5f, y + 0.5f, z + 0.2f }, ::sc2::Colors::Purple);
		}
#endif
		*/

		std::pair<int, int> pylonPos = { 0 , 0 };

		double rampWidth = std::sqrt(std::pow(end1.first - end2.first, 2.0) + std::pow(end1.second - end2.second, 2.0));
		// if (rampWidth < 3) return; 

		// find the first building placment
		std::pair<int, int> pos1 = findNatural3x3Placment(end1, end2, placementGrid);

		if (pos1 != std::pair<int, int>(0, 0))
		{
			_addPlacementPosition(BuildingTypes::BUILDING_3X3, 1, sc2::Point2D(pos1.first + 0.5f, pos1.second + 0.5f), true, true);
			int x_begin = pos1.first - 1;
			int y_begin = pos1.second - 1;
			m_occupied_points.block<3, 3>(x_begin, y_begin).setConstant(1);
		}

		std::cout << "Found placement for first 3x3! " << std::endl;

		if (rampWidth <= 6) pylonPos = findNaturalPylonPlacement({ pos1 }, placementGrid, expansion_locations[1]);

		// find the second building placment
		std::pair<int, int> pos2 = findNatural3x3Placment(end2, end1, placementGrid, pos1);
		if (pos2 != std::pair<int, int>(0, 0))
		{
			_addPlacementPosition(BuildingTypes::BUILDING_3X3, 1, sc2::Point2D(pos2.first + 0.5f, pos2.second + 0.5f), true, true);
			int x_begin = pos2.first - 1;
			int y_begin = pos2.second - 1;

			m_occupied_points.block<3, 3>(x_begin, y_begin).setConstant(1);
		}
		
		std::cout << "Found placement for second 3x3! " << std::endl;

		if (rampWidth <= 9) findNaturalPylonPlacement({ pos1, pos2 }, placementGrid, expansion_locations[1]);

		auto pos3AndBlocker = findNaturalFinal3x3Placement(pos1, pos2, placementGrid, expansion_locations[1]);
		std::pair<int, int> pos3 = pos3AndBlocker.first;
		if (pos3 == std::pair<int, int>(0, 0))
		{
			pylonPos = findNaturalPylonPlacement({ pos1, pos2 }, placementGrid, expansion_locations[1]);
		}
		else
		{
			_addPlacementPosition(BuildingTypes::BUILDING_3X3, 1, sc2::Point2D(pos3.first + 0.5f, pos3.second + 0.5f), true, true);
			int x_begin = pos3.first - 2;
			int y_begin = pos3.second - 2;
			std::cout << x_begin << " " << y_begin << std::endl;
			m_occupied_points.block<5, 5>(x_begin, y_begin).setConstant(1); // no placing within 1 block of the crucial gap

			pylonPos = findNaturalPylonPlacement({ pos1, pos2, pos3 }, placementGrid, expansion_locations[1]);
		}

		std::cout << "Found placement for third 3x3! " << std::endl;

		if (pylonPos != std::pair<int, int>(0, 0))
		{
			_addPlacementPosition(BuildingTypes::BUILDING_2X2, 1, sc2::Point2D(static_cast<float>(pylonPos.first), static_cast<float>(pylonPos.second)), true, true);
			int x_begin = pylonPos.first - 1;
			int y_begin = pylonPos.second - 1;
			m_occupied_points.block<2, 2>(x_begin, y_begin).setConstant(1);
		}

		std::cout << "Found placement for wall pylon at natural! " << std::endl;
	}

	/**
	 * @brief This function was adapted from the burney sc2 library. It identifies clusters of resources
	 * that belong to the same expansion, merges them when they fall within a certain distance threshold
	 * on comparable terrain levels, and then determines valid town-hall placement points for each cluster.
	 */

	std::vector<::sc2::Point2D> PlacementManager::_findExansionLocations()
	{
		if (!m_expansion_locations.empty()) return m_expansion_locations;

		using namespace sc2;
		std::vector<Point2D> final_locations;
		std::vector<Point2D> expansion_locations;
		Units _resources = ManagerMediator::getInstance().GetAllResources(m_bot);
		float distance_squared_threshold = 8.5;

		std::vector<std::vector<const Unit*>> resource_groups;
		resource_groups.reserve(_resources.size());
		for (auto& resource : _resources)
		{
			if (resource->unit_type.ToType() == UNIT_TYPEID::NEUTRAL_MINERALFIELD450) continue;
			resource_groups.push_back({ resource });
		}

		// Merge resource groups if any pair of their resources is closer than our threshold,
		// while ensuring terrain height differences remain acceptable.
		bool merged_group = true;
		while (merged_group)
		{
			merged_group = false;
			for (size_t i = 0; i < resource_groups.size() && !merged_group; ++i)
			{
				for (size_t j = i + 1; j < resource_groups.size() && !merged_group; ++j)
				{
					// Check every pairing across groups i and j.
					bool should_merge = false;
					for (const Unit* resource_a : resource_groups[i])
					{
						for (const Unit* resource_b : resource_groups[j])
						{
							float distance = Distance2D(resource_a->pos, resource_b->pos);
							float height_diff = 
								std::fabs(m_height_map.TerrainHeight(Point2D(resource_a->pos)) - m_height_map.TerrainHeight(Point2D(resource_b->pos)));

							if (distance <= distance_squared_threshold && height_diff < 10.0f)
							{
								should_merge = true;
								break;
							}
						}
						if (should_merge)
						{
							break;
						}
					}
					if (should_merge)
					{
						resource_groups[i].insert(resource_groups[i].end(),
							resource_groups[j].begin(),
							resource_groups[j].end());
						resource_groups.erase(resource_groups.begin() + j);
						merged_group = true;
					}
				}
			}
			// check every combination of two groups
		}

		// Precompute valid offsets around each group's center (within a ring 4 < dist <= 8).
		// This helps to find valid expansions where the town hall can be placed.
		const int offset_range = 7;
		std::vector<Point2D> offsets;
		offsets.reserve((2 * offset_range + 1)* (2 * offset_range + 1));
		for (int x = -offset_range; x <= offset_range; ++x)
		{
			for (int y = -offset_range; y <= offset_range; ++y)
			{
				float dist = std::sqrt(float(x * x + y * y));
				if (dist > 4.0f && dist <= 8.0f)
				{
					offsets.push_back(Point2D(static_cast<float>(x), static_cast<float>(y)));
				}
			}
		}

		std::cout << "Found " << resource_groups.size() << " resource groups" << std::endl;

		// For each group, compute a center and pick the best placement point.
		for (auto& group : resource_groups)
		{
			if (group.empty()) continue;

			// Calculate the geometric center, Add 0.5f so expansion has a .5 coordinate (town halls size 5).
			float sum_x = 0.0f;
			float sum_y = 0.0f;
			for (const Unit* r : group) {
				sum_x += r->pos.x;
				sum_y += r->pos.y;
			}
			int amount = static_cast<int>(group.size());
			float center_x = std::floor(sum_x / amount) + 0.5f;
			float center_y = std::floor(sum_y / amount) + 0.5f;
			Point2D center(center_x, center_y);

			// Generate candidate expansion points within offset ring. 
			// Then pick whichever point is the minimal sum of distances to all resources in the group.
			float best_score = std::numeric_limits<float>::max();
			Point2D best_point;

			for (auto& offset : offsets) {
				Point2D candidate = center + offset;
				Point2D rounded(std::floor(candidate.x + 0.5f), std::floor(candidate.y + 0.5f));

				// Must be buildable at candidate location. 
				// Our placement grid might be a boolean or 1/0 for valid builds.
				// Check placement feasibility.
				if (m_placement_grid.IsPlacable(rounded) == false) continue;

				// Check each resource for minimum required distance (6 for minerals, 7 for geysers).
				bool valid = true;
				for (auto& r : group)
				{
					float min_dist =
						constants::VESPENE_IDS.find(r->unit_type) != constants::VESPENE_IDS.end() ?
						7.0f : 6.0f;
					if (Distance2D(candidate, r->pos) < min_dist)
					{
						valid = false;
						break;
					}
				}
				if (!valid) continue;

				// If we found a valid candidate, score by summing distance to every resource in the group.
				float score_sum = 0.0f;
				for (auto& r : group) {
					score_sum += Distance2D(candidate, r->pos);
				}
				if (score_sum < best_score) {
					best_score = score_sum;
					best_point = candidate;
				}
			}

			// Record the chosen expansion location
			if (best_score < std::numeric_limits<float>::max())
				expansion_locations.push_back(best_point);
		}

		auto* query = m_bot.Query();
		auto* observation = m_bot.Observation();
		auto* debug = m_bot.Debug();

		std::sort(expansion_locations.begin(), expansion_locations.end(), [query, observation](::sc2::Point2D first, ::sc2::Point2D second) {
			return query->PathingDistance(observation->GetStartLocation(), first) < query->PathingDistance(observation->GetStartLocation(), second);
			});

		::sc2::Point2D enemy_base;
		for (const auto& location : expansion_locations)
		{
			if (query->PathingDistance(observation->GetStartLocation(), location) == 0)
			{
				if (location != ::sc2::Point2D(observation->GetStartLocation()))
				{
					enemy_base = location;
					break;
				}
			}
		}

		// Move the enemy base to the end.
		auto it = std::find(expansion_locations.begin(), expansion_locations.end(), enemy_base);
		if (it != expansion_locations.end()) {
			expansion_locations.erase(it); // Remove the enemy base.
			expansion_locations.push_back(enemy_base); // Add it to the end.
		}

		m_expansion_locations = expansion_locations;

		return expansion_locations;
	}

	std::vector<::sc2::Point2D> PlacementManager::_findBuildingLocations(
		const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>& kernel,
		std::tuple<float, float, float, float> bounds,
		size_t xStride,
		size_t yStride,
		const ::sc2::PlacementGrid& placement_grid,
		const Grid& pathing_grid,
		const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> occupied_points,
		size_t buildingWidth,
		size_t buildingHeight)
	{
		if (placement_grid.IsPlacable({ 128, 43 }))
		{
			std::cout << "128, 43 is PLACABLE!!" << std::endl;
		}

		// auto pathing = pathing_grid.GetGrid();
		auto to_avoid = ManagerMediator::getInstance().GetAstarGrid(m_bot).GetGrid();

		// Extract bounds
		int xMin = static_cast<int>(std::round(std::get<0>(bounds)));
		int xMax = static_cast<int>(std::round(std::get<1>(bounds)));
		int yMin = static_cast<int>(std::round(std::get<2>(bounds)));
		int yMax = static_cast<int>(std::round(std::get<3>(bounds)));

		// Dimensions for the region we want to convolve
		int convRows = xMax - xMin + 1;
		int convCols = yMax - yMin + 1;

		// Create a matrix of ones (uint8_t(1))
		Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> toConvolve(convRows, convCols);
		toConvolve.setOnes();

		// Fill toConvolve according to the conditions
		// Note that indexing is toConvolve(x - xMin, y - yMin)
		for (unsigned int i = xMin; i <= xMax; ++i) {
			for (unsigned int j = yMin; j <= yMax; ++j) {
				// Check the logic from the original snippet
				if (occupied_points(i, j) == 0 &&
					placement_grid.IsPlacable({static_cast<int>(i), static_cast<int>(j)}) &&
					to_avoid(j, i) == 1)
				{
					toConvolve(i - xMin, j - yMin) = 0;
				}
			}
		}

		// convolve
		Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> result = convolve2dValid(toConvolve, kernel);

		// Prepare a place to store valid spots
		std::vector<::sc2::Point2D> valid_points;
		valid_points.reserve(500);

		float half_width = static_cast<float>(buildingWidth) / 2.0f;

		// skip every 4th location on the same row
		// skip entire "j" row if blocked
		std::set<unsigned int> blockedY;

		for (int i = 0; i < result.rows(); i += xStride)
		{
			unsigned int found_this_many_on_y = 0;

			for (int j = 0; j < result.cols(); j += yStride)
			{
				if (result(i, j) == 0)
				{
					// This row is blocked
					if (blockedY.find(j) != blockedY.end()) continue;
					found_this_many_on_y += 1;
					// lf 4 are found in a row, block the row
					if (j > 0 && (found_this_many_on_y % 4 == 0))
					{
						blockedY.insert(j);
						continue;
					}

					float xCenter = static_cast<float>(i + xMin) + half_width;
					float yCenter = static_cast<float>(j + yMin) + half_width;

					valid_points.emplace_back(xCenter, yCenter);
				}
			}
		}

		return valid_points;
	}

	// Naive 2D "valid" convolution, returning an Eigen::Matrix<uint8_t, Dynamic, Dynamic>
	Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>
		PlacementManager::convolve2dValid(
			const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>& input,
			const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>& kernel)
	{
		// Dimensions
		const int inRows = input.rows();
		const int inCols = input.cols();
		const int kRows = kernel.rows();
		const int kCols = kernel.cols();

		// Output dimensions (valid convolution)
		const int outRows = inRows - kRows + 1;
		const int outCols = inCols - kCols + 1;

		// Create the result matrix
		Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic> result(outRows, outCols);
		result.setZero();  // Initialize to 0

		// Naive convolution
		for (int i = 0; i < outRows; ++i) {
			for (int j = 0; j < outCols; ++j) {
				// Accumulate the sum for (i, j)
				int sum = 0; // or use int16_t / int32_t if your kernel can exceed uint8_t range
				for (int ki = 0; ki < kRows; ++ki) {
					for (int kj = 0; kj < kCols; ++kj) {
						sum += input(i + ki, j + kj) * kernel(ki, kj);
					}
				}
				// Cast or clamp as needed if you worry about overflow
				result(i, j) = static_cast<uint8_t>(sum);
			}
		}
		return result;
	}

	std::optional<::sc2::Point2D> PlacementManager::_requestBuildingPlacement(int base_number, ::sc2::UNIT_TYPEID structure_type,
		bool is_wall,
		bool find_alternative,
		bool reserve_placement,
		bool within_power_field,
		float pylon_build_progress,
		bool build_close_to,
		::sc2::Point2D close_to)
	{
		::sc2::Point2D result;
		size_t at_base = base_number;

		BuildingTypes building_type = _structureToBuildingSize(structure_type);
		if (building_type == NOT_FOUND) return std::nullopt;

		auto available_positions = _findPotentialPlacementsAtBase(building_type, base_number, within_power_field, pylon_build_progress);

		if (available_positions.empty())
		{
			if (!find_alternative)
			{
				std::cout << "Not available position for building size near base no. " << base_number << std::endl;
				return std::nullopt;
			}
			else
			{
				for (int i = base_number + 1; i <= 8; ++i)
				{
					at_base = i;
					available_positions = _findPotentialPlacementsAtBase(building_type, at_base, within_power_field, pylon_build_progress);
					if (!available_positions.empty()) break;
				}
				std::cout << "Not available position for building size anywhere on the map for " << ::sc2::UnitTypeToName(structure_type) << std::endl;
				return std::nullopt;
			}
		}

		if (!build_close_to)
		{
			result = utils::GetClosestTo(_findExansionLocations()[base_number], available_positions);
		}
		else
		{
			result = utils::GetClosestTo(close_to, available_positions);
		}

		if (is_wall)
		{
			for (const auto& position : available_positions)
			{
				if (m_expansion_map[base_number][building_type][{position.x, position.y}].is_wall)
				{
					result = position;
					break;
				}
			}
		}
		else if (structure_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON)
		{
			std::vector<::sc2::Point2D> optimal_pylons;
			for (const auto& position : available_positions)
			if (!m_expansion_map[base_number][building_type][{position.x, position.y}].is_wall)
			{
				if (m_expansion_map[base_number][building_type][{position.x, position.y}].optimal_pylon)
				{
					optimal_pylons.push_back(position);
					continue;
				}
				else if (m_expansion_map[base_number][building_type][{position.x, position.y}].production_pylon)
				{
					optimal_pylons.push_back(position);
				}
			}
			if (!optimal_pylons.empty()) result = utils::GetClosestTo(_findExansionLocations()[base_number], optimal_pylons);
		}

		if (reserve_placement)
		{
			m_expansion_map[at_base][building_type][{result.x, result.y}].worker_on_route = true;
			m_expansion_map[at_base][building_type][{result.x, result.y}].time_requested = m_bot.Observation()->GetGameLoop();
		}

		return result;
	}

	std::vector<::sc2::Point2D> PlacementManager::_findPotentialPlacementsAtBase(BuildingTypes building_size, 
		int base_index,
		bool within_power_field, 
		float pylon_build_progress)
	{
		std::vector<::sc2::Point2D> result;
		auto potential_placements = m_expansion_map[base_index][building_size];
		// std::cout << "PlacementManager: searching base index " << base_index << " for" << building_size << std::endl;
		::sc2::Units own_pylons;

		if (within_power_field)
		{
			::sc2::Units structures = ManagerMediator::getInstance().GetAllOwnStructures(m_bot);
			for (const auto& structure : structures)
			{
				if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON)
				{
					if (structure->build_progress >= pylon_build_progress)
					{
						own_pylons.push_back(structure);
					}
				}
			}
			if (own_pylons.empty()) return result;

			// find positions cloe to pylons

		}

		for (const auto& candidate : potential_placements)
		{
			if (candidate.second.available && !candidate.second.worker_on_route)
			{
				::sc2::Point2D pos = { candidate.first.first, candidate.first.second };
				if (_canPlaceStructure(pos, building_size)) result.push_back(pos);
			}
		}

		if (within_power_field)
		{
			std::vector<::sc2::Point2D> placements_within_power_field;
			for (const auto& placement : result)
				if (utils::isPowered(placement, own_pylons, m_height_map)) 
					placements_within_power_field.push_back(placement);
			result = placements_within_power_field;
		}

		// std::cout << "PlacementManager: out of the potential placements there are " << potential_placements.size() << "available placements. " << std::endl;
		return result;
	}

	bool PlacementManager::_canPlaceStructure(::sc2::Point2D position, BuildingTypes building_size, bool is_geyser)
	{
		if (is_geyser)
		{
			return false;
		}
		float building_radius;
		if (building_size == BUILDING_2X2) building_radius = 1.0f;
		else if (building_size == BUILDING_3X3) building_radius = 1.5f;
		else if (building_size == BUILDING_5X5) building_radius = 2.5f;
		else return false;
		
		int start_x = static_cast<int>(std::round(position.x - building_radius));
		int start_y = static_cast<int>(std::round(position.y - building_radius));
		int size = static_cast<int>(std::round(building_radius * 2));

		return utils::canPlaceStructure(start_x, start_y, size, m_placement_grid);
	}
		

	BuildingTypes PlacementManager::_structureToBuildingSize(::sc2::UNIT_TYPEID structure_id)
	{
		switch (structure_id)
		{
		// 2x2s
		case (::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON): return BUILDING_2X2;
		case (::sc2::UNIT_TYPEID::PROTOSS_PYLON): return BUILDING_2X2;
		case (::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY): return BUILDING_2X2;
		case (::sc2::UNIT_TYPEID::PROTOSS_DARKSHRINE): return BUILDING_2X2;
		// 3x3s
		case (::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_FLEETBEACON): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_FORGE): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_GATEWAY): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_STARGATE): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE): return BUILDING_3X3;
		case (::sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL): return BUILDING_3X3;
		// 5x5
		case (::sc2::UNIT_TYPEID::PROTOSS_NEXUS): return BUILDING_5X5;
		default: return NOT_FOUND;
		}
	}

	void PlacementManager::_addPlacementPosition(
		BuildingTypes building_type,
		int expansion_location,
		::sc2::Point2D position,
		bool available,
		bool is_wall,
		int building_tag,
		bool worker_on_route,
		double time_requested,
		bool production_pylon,
		bool optimal_pylon)
	{
		m_expansion_map[expansion_location][building_type][{position.x, position.y}] = 
			BuildingAttributes(available, is_wall, building_tag, worker_on_route, time_requested, production_pylon, optimal_pylon);
	}
}