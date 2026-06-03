#include "scouting_manager.h"

#include "manager.h"
#include "manager_mediator.h"
#include "../../utils/position_utils.h"
#include "../../utils/unit_utils.h"
#include "../../pathing/grid.h"
#include "../../Aeolus.h"
#include <tuple>
#include <unordered_map>
#include <any>
#include <sc2api/sc2_map_info.h>


namespace Aeolus 
{
	std::any ScoutingManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case constants::ManagerRequestType::REGISTER_SCOUT:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*, std::vector<int>>>(args);
			const ::sc2::Unit* scoutUnit = std::get<0>(params);
			std::vector<int> basesToScout = std::get<1>(params);
			return _registerScout(scoutUnit, basesToScout);
		}
		case constants::ManagerRequestType::CHECK_SCOUT_TO_NEXT_WAYPOINT:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* scoutUnit = std::get<0>(params);
			return _checkScoutToNextWaypoint(scoutUnit);
		}
		case constants::ManagerRequestType::GET_OPPONENT_RACE:
			return _getOpponentRace();
		case constants::ManagerRequestType::GET_BEHIND_MINERAL_LOCATIONS:
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D>>(args);
			::sc2::Point2D baseLocation = std::get<0>(params);
			return _getBehindMineralPositions(baseLocation);
		}
		default:
			return 0;
		}
	}

	void ScoutingManager::update(int iteration)
	{
	}

	void ScoutingManager::Initialize()
	{
		auto& mediator = ManagerMediator::getInstance();
		for (const auto& expansionLocation : mediator.GetExpansionLocations(m_bot))
		{
			m_behind_mineral_pos_cache.push_back(_getBehindMineralPositions(expansionLocation));
		}
	}

	std::vector<::sc2::Point2D> ScoutingManager::_getBehindMineralPositions(::sc2::Point2D expansion_location)
	{
		auto& mediator = ManagerMediator::getInstance();

		::sc2::Units mineralFields = mediator.GetAllMineralPatches(m_bot);
		::sc2::Units closeMineralFields = utils::GetCloserThan(mineralFields, 10.0f, expansion_location);

		std::vector<::sc2::Point2D> result;

		if (closeMineralFields.size() >= 2)
		{
			// get the center pos of all mineral fields
			float tot_x = 0.0f, tot_y = 0.0f;
			size_t sizeMineralFields = closeMineralFields.size();
			for (const auto* mineralField : closeMineralFields)
			{
				tot_x += mineralField->pos.x;
				tot_y += mineralField->pos.y;
			}
			::sc2::Point2D centerMf(tot_x / sizeMineralFields, tot_y / sizeMineralFields);
			auto furthestMfs = utils::SortByDistanceTo(closeMineralFields, centerMf, true);
			result.push_back(utils::GetPositionTowards(
				expansion_location, 
				::sc2::Point2D(furthestMfs[0]->pos.x, furthestMfs[0]->pos.y), 
				9.0f));
			result.push_back(utils::GetPositionTowards(
				expansion_location,
				centerMf,
				9.0f));
			result.push_back(utils::GetPositionTowards(
				expansion_location,
				::sc2::Point2D(furthestMfs[1]->pos.x, furthestMfs[1]->pos.y),
				9.0f));
		}
		else
		{
			// give up if something goes wrong lol
			::sc2::Point2D pMax = m_bot.Observation()->GetGameInfo().playable_max;
			::sc2::Point2D pMin = m_bot.Observation()->GetGameInfo().playable_min;
			::sc2::Point2D mapCenter((pMax.x + pMin.x) / 2, (pMax.y + pMin.y) / 2);
			result.push_back(utils::GetPositionTowards(expansion_location, mapCenter, 5.0f));
			result.push_back(utils::GetPositionTowards(expansion_location, mapCenter, 5.0f));
			result.push_back(utils::GetPositionTowards(expansion_location, mapCenter, 5.0f));
		}
		return result;
	}

	bool ScoutingManager::_registerScout(const ::sc2::Unit* scoutUnit, std::vector<int> expansionsToScout)
	{
		std::queue<::sc2::Point2D> posQueue;
		for (const auto& exp : expansionsToScout)
		{
			for (const auto& pos : m_behind_mineral_pos_cache[exp])
			{
				posQueue.push(pos);
			}
		}
		m_scouting_paths.insert({ scoutUnit->tag, posQueue });
		return true;
	}

	::sc2::Point2D ScoutingManager::_checkScoutToNextWaypoint(const ::sc2::Unit* scoutUnit)
	{
		auto& mediator = ManagerMediator::getInstance();

		auto& wayPointQueue = m_scouting_paths[scoutUnit->tag];
		
		if (wayPointQueue.empty())
		{
			// scout unit has cleared all the waypoints, it has fulfilled
			// its duty and is no longer a scout unit
			mediator.AssignRole(m_bot, scoutUnit, constants::UnitRole::GATHERING);
			return ::sc2::Point2D({ 0.0f, 0.0f });
		}

		auto pathingGrid = ::sc2::PathingGrid(m_bot.Observation()->GetGameInfo());
		int goal_x = static_cast<int>(std::round(wayPointQueue.front().x));
		int goal_y = static_cast<int>(std::round(wayPointQueue.front().y));
		auto pathable = pathingGrid.IsPathable({ goal_x, goal_y });

		bool reachedNextWayPoint = ::sc2::DistanceSquared2D(scoutUnit->pos, wayPointQueue.front()) < 1.0f;
		bool isStaticScout = false;
		if (m_last_seen.find(scoutUnit->tag) != m_last_seen.end())
		{
			isStaticScout = utils::Point2Dcmp(m_last_seen[scoutUnit->tag], scoutUnit->pos);
		}

		if (reachedNextWayPoint || !pathable || isStaticScout)
		{
			// scout unit is reaching the next available waypoint, or the waypoint is not pathable, or if it has not moved (probably stuck)
			// move to the next waypoint
			wayPointQueue.pop();
			if (wayPointQueue.empty()) {
				_clearScout(scoutUnit);
				return { 0.0f, 0.0f };
			}
		}

		m_last_seen[scoutUnit->tag] = scoutUnit->pos;

		return wayPointQueue.front();
	}

	void ScoutingManager::_clearScout(const ::sc2::Unit* unit)
	{
		auto& mediator = ManagerMediator::getInstance();
		mediator.AssignRole(m_bot, unit, constants::UnitRole::GATHERING);
		m_scouting_paths.erase(unit->tag);
		m_last_seen.erase(unit->tag);
	}

	::sc2::Race ScoutingManager::_getOpponentRace()
	{
		const auto own_id = m_bot.Observation()->GetPlayerID();

		::sc2::Race opponentRace = ::sc2::Race::Random;
		for (const auto& player : m_bot.Observation()->GetGameInfo().player_info) {
			if (player.player_id != own_id) {
				opponentRace = player.race_requested;
			}
		}

		return opponentRace;
	}

} // namespace aeolus