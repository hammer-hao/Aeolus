#pragma once

#include "manager.h"
#include "../pathing/map_data.h"
#include "../pathing/grid.h"

#include <sc2api/sc2_common.h>

namespace Aeolus
{
	class AeolusBot;
	class MapData;

	class PathManager : public Manager
	{
	public:
		// constructor will take AeolusBot object reference
		PathManager(AeolusBot& aeolusbot)
			: m_mapdata(aeolusbot), m_bot(aeolusbot), m_danger_tiles_is_cached(false)
		{
			std::cout << "Path manager initialization at address: " << this << std::endl;
		}

		std::string_view GetName() const override {
			static const std::string name = "PathManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		// Implement the update method.
		void update(int iteration) override;

		void AddUnitInfluence(const ::sc2::Unit* enemy);

	private:
		MapData m_mapdata;
		AeolusBot& m_bot;
		Grid m_ground_grid;
		std::vector<std::pair<int, int>> m_danger_tiles_cache;
		bool m_danger_tiles_is_cached;

		void _addUnitInfluence(const ::sc2::Unit* unit);

		bool _isGroundPositionSafe(::sc2::Point2D position);

		void _reset_grids(); // reset all grids to cached versions

		void _reset_danger_tiles(); // reset danger tiles cache to empty

		::sc2::ImageData _getDefaultGridData();

		Grid _getAStarGrid();

		::sc2::Point2D _getClosestSafeSpot(::sc2::Point2D position, const double& radius);

		std::vector<::sc2::Point2D> _getFloodFillArea(::sc2::Point2D starting_point, int max_distance);

		/*
		@brief Given a start point, a goal point, and a grid with enemy influence,
		find the shortest safe path from start point to the goal point and returns
		the next point to move to in that path.
		*/
		::sc2::Point2D AStarPathFindNext(::sc2::Point2D start, ::sc2::Point2D goal,
			const Grid& grid, bool sense_danger = true, int danger_distance = 20,
			float danger_threshold = 5.0f, bool smoothing = false, int sensitivity = 5);
	};
}