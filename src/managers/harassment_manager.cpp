#include "harassment_manager.h"
#include "manager_mediator.h"

#include "../Aeolus.h"
#include <sc2api/sc2_map_info.h>

namespace Aeolus
{
	std::any HarassmentManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_POSITIONS_BEHIND_ENEMY_MAIN_NATURAL_THIRD):
		{
			return _getPositionBehindEnemyMainNaturalThird();
		}
		case (constants::ManagerRequestType::GET_HARASSMENT_TRACKER):
		{
			return _getHarassmentTracker();
		}
		case (constants::ManagerRequestType::REGISTER_HARASSMENT_STATUS):
		{
			auto params = std::any_cast<std::tuple<::sc2::Tag, HarassmentStatus>>(args);
			::sc2::Tag unitTag = std::get<0>(params);
			HarassmentStatus status = std::get<1>(params);
			_registerHarassmentStatus(unitTag, status);
			return 0;
		}
		default:
			return 0;
		}
	}
	void HarassmentManager::Initialize()
	{
		auto& mediator = ManagerMediator::getInstance();

		std::vector<::sc2::Point2D> expansions = mediator.GetExpansionLocations(m_bot);
		expansions.erase(expansions.begin()); // don't count our own start location

		::sc2::Point2D enemyMainBaseLocation = expansions.back();

		auto* query = m_bot.Query();
		auto* observation = m_bot.Observation();
		auto* debug = m_bot.Debug();

		std::sort(expansions.begin(), expansions.end(), [query, observation, enemyMainBaseLocation](::sc2::Point2D first, ::sc2::Point2D second) {
			return query->PathingDistance(enemyMainBaseLocation, first) < query->PathingDistance(enemyMainBaseLocation, second);
		});

		while (expansions.size() < 3) {
			expansions.push_back(expansions.back());
		}

		for (int i = 0; i < 3; ++i)
		{
			std::vector<::sc2::Point2D> behindMineralPositions = mediator.getBehindMineralPositions(m_bot, expansions[i]);

			m_position_behind_enemy_main_natural_third.push_back(behindMineralPositions);
		}
	}

	void HarassmentManager::update(int iteration)
	{
#ifndef BUILD_FOR_LADDER
		/*::sc2::HeightMap heightmap(m_bot.Observation()->GetGameInfo());
		auto* debug = m_bot.Debug();

		for (const auto& locations : m_position_behind_enemy_main_natural_third)
		{
			for (const auto& location : locations)
			{
				float z = heightmap.TerrainHeight(location);
				debug->DebugSphereOut(::sc2::Point3D(location.x, location.y, z), 1.0, ::sc2::Colors::Blue);
			}
		}
		debug->SendDebug();*/
#endif
	}

	void HarassmentManager::onUnitDestroyed(const ::sc2::Unit* unit)
	{
		m_harassment_tracker.erase(unit->tag);
	}

	std::vector<std::vector<::sc2::Point2D>> HarassmentManager::_getPositionBehindEnemyMainNaturalThird()
	{
		return m_position_behind_enemy_main_natural_third;
	}

	std::map<::sc2::Tag, HarassmentStatus> HarassmentManager::_getHarassmentTracker()
	{
		return m_harassment_tracker;
	}

	void HarassmentManager::_registerHarassmentStatus(::sc2::Tag unitTag, HarassmentStatus status)
	{
		m_harassment_tracker[unitTag] = status;
	}
}