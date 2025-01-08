#include "placement_manager.h"
#include "manager_mediator.h"
#include "../pathing/grid.h"
#include "../Aeolus.h"
#include <chrono>
#include <sc2lib/sc2_search.h>
#include <sc2api/sc2_map_info.h>
#include <Eigen/Dense>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <cmath>

namespace Aeolus
{
	std::any PlacementManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		default: return 0;
		}
	}

	void PlacementManager::update(int iteration)
	{

	}

	void PlacementManager::Initialize()
	{
		auto start = std::chrono::high_resolution_clock::now();

		//solve placement logic

		Grid placement_grid = Grid(m_bot.Observation()->GetGameInfo().placement_grid);
		Grid pathing_grid = Grid(m_bot.Observation()->GetGameInfo().pathing_grid);
		const auto height_map = ::sc2::HeightMap(m_bot.Observation()->GetGameInfo());

		m_occupied_points.resize(placement_grid.GetWidth(), placement_grid.GetHeight()); // (occupied points is x, y)
		m_occupied_points.setZero();

		std::vector<::sc2::Point2D> expansion_locations;
		expansion_locations.push_back(::sc2::Point2D(m_bot.Observation()->GetStartLocation()));

		auto* debug = m_bot.Debug();

		for (const auto& location : _findExansionLocations(height_map, placement_grid))
		{
			float z = height_map.TerrainHeight(location);
			debug->DebugSphereOut(sc2::Point3D(location.x, location.y, z), 
				1.0, 
				::sc2::Colors::Red);
			expansion_locations.push_back(sc2::Point2D(location));
		}

		_calculateProtossRampPylonPos(::sc2::Point2D(m_bot.Observation()->GetStartLocation()), pathing_grid, placement_grid, height_map);

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << "Completed placement calculation in " << duration.count() << " ms." << std::endl;
	}

	::sc2::Point2D PlacementManager::_calculateProtossRampPylonPos(::sc2::Point2D main_location,
			const Grid& pathing_grid, const Grid& placement_grid, const ::sc2::HeightMap& height_map)
	{
		std::vector<std::vector<std::pair<int, int>>> ramp_clusters;
		std::set<std::pair<int, int>> unvisited;

		Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> pathing_matrix = pathing_grid.GetGrid();
		Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> placement_matrix = placement_grid.GetGrid();

		// 1) Fill 'unvisited' with pathable && not placable points
		for (int y = 0; y < pathing_grid.GetHeight(); ++y) {
			for (int x = 0; x < pathing_grid.GetWidth(); ++x) {
				if (pathing_matrix(y, x) == 1 && placement_matrix(y , x) == 0) 
				{
					unvisited.insert({x, y});
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
				float terrain_height = height_map.TerrainHeight(::sc2::Point2D(p.first, p.second));
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

		std::vector<std::pair<int, int>>& main_ramp_cluster = ramp_clusters[best_index];

		auto* debug = m_bot.Debug();

		for (const auto& point : main_ramp_cluster)
		{
			float z = height_map.TerrainHeight(::sc2::Point2D(point.first, point.second));
			debug->DebugSphereOut(::sc2::Point3D(point.first, point.second, z), 0.5, ::sc2::Colors::Green);
		}

		// 2) Identify the ramp's "upper" points
		//    (the ones matching the maximum terrain height within the ramp).
		float maxHeight = -9999.0f;
		std::vector<std::pair<int, int>> rampUpperPoints;
		for (auto& pt : main_ramp_cluster) 
		{
			float terrain_height = height_map.TerrainHeight(::sc2::Point2D(pt.first, pt.second));
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

		float UpperxSum = 0, UpperySum = 0;
		for (const auto& point : rampUpperPoints)
		{
			float z = height_map.TerrainHeight(::sc2::Point2D(point.first, point.second));
			debug->DebugSphereOut(::sc2::Point3D(point.first, point.second, z), 0.5, ::sc2::Colors::Red);
			UpperxSum += point.first;
			UpperySum += point.second;
		}
		::sc2::Point2D upper_middle = { UpperxSum / rampUpperPoints.size(), UpperySum / rampUpperPoints.size() };
		debug->DebugSphereOut(::sc2::Point3D(upper_middle.x, upper_middle.y, height_map.TerrainHeight(upper_middle)), 0.5, ::sc2::Colors::Purple);

		// 3) Compute the ramp's bottom center by averaging the lowest points.
		std::vector<std::pair<int, int>> rampLowerPoints;
		float minHeight = 9999.0f;
		for (auto& pt : main_ramp_cluster) {
			float h = height_map.TerrainHeight(::sc2::Point2D(pt.first, pt.second));
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
			float z = height_map.TerrainHeight(::sc2::Point2D(point.first, point.second));
			debug->DebugSphereOut(::sc2::Point3D(point.first, point.second, z), 0.5, ::sc2::Colors::Blue);
			lowerxSum += point.first;
			lowerySum += point.second;
		}
		::sc2::Point2D lower_middle = { lowerxSum / rampLowerPoints.size(), lowerySum / rampLowerPoints.size() };
		debug->DebugSphereOut(::sc2::Point3D(lower_middle.x, lower_middle.y, height_map.TerrainHeight(lower_middle)), 0.5, ::sc2::Colors::Purple);

		::sc2::Point2D ramp_direction = (upper_middle - lower_middle) / ::sc2::Distance2D(upper_middle, lower_middle);
		::sc2::Point2D pylon_position_float = upper_middle + ramp_direction * 6.0f;
		std::pair<int,int> pylon_position = { static_cast<int>(std::round(pylon_position_float.x)), static_cast<int>(std::round(pylon_position_float.y)) };

		debug->DebugSphereOut(::sc2::Point3D(pylon_position.first, pylon_position.second, height_map.TerrainHeight(pylon_position_float)), 0.5, ::sc2::Colors::Purple);

		std::cout << "Final pylon position: " << pylon_position.first << " " << pylon_position.second << std::endl;

		::sc2::Point2D building1_position_float = upper_middle + ramp_direction * 2.5f;
		debug->DebugSphereOut(::sc2::Point3D(building1_position_float.x, building1_position_float.y, height_map.TerrainHeight(building1_position_float)), 0.5, ::sc2::Colors::Purple);
		
		debug->SendDebug();

		_addPlacementPosition(BuildingTypes::BUILDING_2X2, 0, pylon_position_float, true, true);

		return main_location;
	}

	/**
	 * @brief This function was adapted from the burney sc2 library. It identifies clusters of resources
	 * that belong to the same expansion, merges them when they fall within a certain distance threshold
	 * on comparable terrain levels, and then determines valid town-hall placement points for each cluster.
	 */

	std::vector<::sc2::Point2D> PlacementManager::_findExansionLocations(const ::sc2::HeightMap& height_map, const Grid& placement_grid)
	{
		using namespace sc2;
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
								std::fabs(height_map.TerrainHeight(Point2D(resource_a->pos)) - height_map.TerrainHeight(Point2D(resource_b->pos)));

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
		offsets.reserve((2 * offset_range + 1) * (2 * offset_range + 1));
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
				if (placement_grid.GetGrid()(static_cast<int>(rounded.y), static_cast<int>(rounded.x)) == 0) continue;

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

		return expansion_locations;
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