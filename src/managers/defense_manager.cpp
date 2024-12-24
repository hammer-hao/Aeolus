#include "defense_manager.h"
#include "nanoflann.hpp"
#include "manager_mediator.h"
#include "../constants.h"
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
			auto params = std::any_cast<std::tuple<std::vector<StartPointType>, float>>(args);
			std::vector<StartPointType> starting_points = std::get<0>(params);
			float distance = std::get<1>(params);
			return UnitsInRange(starting_points, distance, *m_all_enemy_units_tree);
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
		nanoflann::SearchParams params;

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
				std::vector<std::pair<uint32_t, float>> matches;
				tree.tree->radiusSearch(query_point, distance, matches, params);

				for (const auto& match : matches)
				{
					const ::sc2::Unit* unit = tree.unit_map[match.first];
					unique_units.insert(unit);
				}
			}
			units_in_range.assign(unique_units.begin(), unique_units.end());
		}
		return units_in_range;
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