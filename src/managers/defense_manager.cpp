#include "defense_manager.h"
#include "../thirdparty/nanoflann.hpp"
#include "manager_mediator.h"
#include "../constants.h"
#include "../utils/kdtree_utils.h"
#include <sc2api/sc2_unit.h>
#include <unordered_set>

namespace Aeolus
{
	void DefenseManager::update(int iteration)
	{
		if (iteration > 0)
		{
			m_own_town_halls = ManagerMediator::getInstance().GetOwnTownHalls(m_bot);
			GenerateKDTrees();
		}
	}
	std::any DefenseManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_UNITS_IN_RANGE):
		{
			auto params = std::any_cast<std::tuple<std::vector<::sc2::Point2D>, float>>(args);
			std::vector<::sc2::Point2D> starting_points = std::get<0>(params);
			float distance = std::get<1>(params);

			// Convert that std::vector<::sc2::Point2D> into std::vector<StartPointType>
			std::vector<StartPointType> st_points;
			st_points.reserve(starting_points.size());
			for (auto& pt : starting_points)
			{
				// Since StartPointType = std::variant<const sc2::Unit*, sc2::Point2D>
				// we can push_back the Point2D directly into that variant
				st_points.push_back(pt);
			}

			return UnitsInRange(st_points, distance, *m_all_enemy_units_tree);
		}
		case (constants::ManagerRequestType::GET_OWN_UNITS_IN_RANGE):
		{
			auto params = std::any_cast<std::tuple<std::vector<::sc2::Point2D>, float>>(args);
			std::vector<::sc2::Point2D> starting_points = std::get<0>(params);
			float distance = std::get<1>(params);
			// Convert that std::vector<::sc2::Point2D> into std::vector<StartPointType>
			std::vector<StartPointType> st_points;
			st_points.reserve(starting_points.size());
			for (auto& pt : starting_points)
			{
				// Since StartPointType = std::variant<const sc2::Unit*, sc2::Point2D>
				// we can push_back the Point2D directly into that variant
				st_points.push_back(pt);
			}
			return UnitsInRange(st_points, distance, *m_all_own_units_tree);
		}
		case (constants::ManagerRequestType::GET_ENEMY_UNITS_IN_RANGE_MAP):
		{
			auto params = std::any_cast<std::tuple<std::vector<::sc2::Point2D>, float>>(args);
			std::vector<::sc2::Point2D> starting_points = std::get<0>(params);
			float distance = std::get<1>(params);

			// Convert that std::vector<::sc2::Point2D> into std::vector<StartPointType>
			std::vector<StartPointType> st_points;
			st_points.reserve(starting_points.size());
			for (auto& pt : starting_points)
			{
				// Since StartPointType = std::variant<const sc2::Unit*, sc2::Point2D>
				// we can push_back the Point2D directly into that variant
				st_points.push_back(pt);
			}

			return UnitsInRangeMap(st_points, distance, *m_all_enemy_units_tree);
		}
		case (constants::ManagerRequestType::GET_GROUND_THREATS_NEAR_BASES):
		{
			return MainThreatsNearTownHall();
		}
		default: return 0;
		}
	}

	void DefenseManager::GenerateKDTrees()
	{
		::sc2::Units all_enemy_units = ManagerMediator::getInstance().GetAllEnemyUnits(m_bot);
		::sc2::Units all_own_units = ManagerMediator::getInstance().GetAllOwnUnits(m_bot);
		m_all_enemy_units_tree = UnitsKDTree::create(all_enemy_units);
		m_all_own_units_tree = UnitsKDTree::create(all_own_units);
	}

	::sc2::Units DefenseManager::UnitsInRange(
		const std::vector<StartPointType>& starting_points,
		float distance,
		UnitsKDTree& tree)
	{
		// Result list to hold units in range for each start point
		::sc2::Units units_in_range;
		std::unordered_set<const ::sc2::Unit*> unique_units;
		nanoflann::SearchParameters params;

		if (!starting_points.empty())
		{
			std::vector<::sc2::Point2D> positions;
			positions.reserve(starting_points.size());
			for (const auto& point : starting_points)
			{
				if (std::holds_alternative<const ::sc2::Unit*>(point))
				{
					const ::sc2::Unit* unit = std::get<const ::sc2::Unit*>(point);
					positions.emplace_back(unit->pos.x, unit->pos.y);
				}
				else
				{
					positions.emplace_back(std::get<::sc2::Point2D>(point));
				}
			}

			// Query the KDTree for units within range
			for (const auto& position : positions)
			{
				std::vector<size_t> indices; // to hold query results;

				// perform range search
				float query_point[2] = { position.x, position.y };
				std::vector<nanoflann::ResultItem<unsigned int, float>> search_results;
				if (tree.tree) tree.tree->radiusSearch(query_point, distance * distance, search_results, params);
				// else std::cout << "No existing tree!" << std::endl;

				for (const auto& result : search_results)
				{
					const ::sc2::Unit* unit = tree.unit_map[result.first];
					unique_units.insert(unit);
				}
			}
			if (unique_units.size() > 0) units_in_range.assign(unique_units.begin(), unique_units.end());
		}
		return units_in_range;
	}

	std::vector<::sc2::Units> DefenseManager::UnitsInRangeMap(
		const std::vector<StartPointType>& starting_points,
		float distance,
		UnitsKDTree& tree)
	{
		// We'll return a vector of the same size as 'starting_points',
		// where each element is the list of units in range of that start point.
		std::vector<::sc2::Units> results(starting_points.size());

		nanoflann::SearchParameters params;

		if (!starting_points.empty() && tree.tree) {
			// Loop through each starting point
			for (size_t i = 0; i < starting_points.size(); ++i) {
				// Determine the position (x, y) for this start point
				const auto& point = starting_points[i];
				::sc2::Point2D position;
				if (std::holds_alternative<const ::sc2::Unit*>(point)) {
					position.x = std::get<const ::sc2::Unit*>(point)->pos.x;
					position.y = std::get<const ::sc2::Unit*>(point)->pos.y;
				}
				else {
					position = std::get<::sc2::Point2D>(point);
				}

				// Prepare for the radius search
				float query_point[2] = { position.x, position.y };
				std::vector<nanoflann::ResultItem<unsigned int, float>> search_results;

				// Perform range search for this single start point
				tree.tree->radiusSearch(query_point, distance * distance, search_results, params);

				// Collect unique units for this start point
				std::unordered_set<const ::sc2::Unit*> unique_units;
				for (const auto& result : search_results) {
					const ::sc2::Unit* unit = tree.unit_map[result.first];
					unique_units.insert(unit);
				}

				// Move into the results vector element for this start point
				if (!unique_units.empty()) {
					results[i].reserve(unique_units.size());
					results[i].assign(unique_units.begin(), unique_units.end());
				}
			}
		}

		return results;
	}

	::sc2::Units DefenseManager::MainThreatsNearTownHall()
	{
		std::vector<StartPointType> starting_points;
		starting_points.reserve(m_own_town_halls.size());

		// Assuming StartPointType can hold a const ::sc2::Unit*
		for (const auto* town_hall : m_own_town_halls) {
			starting_points.push_back(town_hall);  // Directly pushing as StartPointType, if compatible
		}
		return UnitsInRange(starting_points, m_ground_threat_range, *m_all_enemy_units_tree);
	}
}