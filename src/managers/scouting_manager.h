#pragma once

#include "manager.h"
#include <tuple>
#include <unordered_map>
#include <queue>
#include <any>

namespace Aeolus
{
	class AeolusBot;

	/**
	* Manager for keeping track of scouting units and their scout paths
	*/
	class ScoutingManager : public Manager
	{
	public:
		ScoutingManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
		}

		std::string_view GetName() const override {
			static const std::string name = "ScoutingManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

		void Initialize();

	private:
		AeolusBot& m_bot;

		std::unordered_map<::sc2::Tag, std::queue<::sc2::Point2D>> m_scouting_paths;

		std::unordered_map<::sc2::Tag, sc2::Point2D> m_last_seen;

		std::vector<std::vector<::sc2::Point2D>> m_behind_mineral_pos_cache;

		std::vector<::sc2::Point2D> _getBehindMineralPositions(::sc2::Point2D expansion_location);

		bool _registerScout(const ::sc2::Unit* scoutUnit, std::vector<int> expansionsToScout);

		::sc2::Point2D _checkScoutToNextWaypoint(const ::sc2::Unit* scoutUnit);

		void _clearScout(const ::sc2::Unit* unit);

		::sc2::Race _getOpponentRace();

		std::optional<::sc2::Race> m_opponent_race;
	};
}