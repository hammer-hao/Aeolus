#pragma once

#include "grid.h"
#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2lib/sc2_search.h>

#include <Eigen/Dense>
#include <map>
#include <tuple>

namespace Aeolus
{

	class AeolusBot;
	class MapData
	{
	public:
		MapData(AeolusBot& aeolusbot);

		Grid GetAStarGrid(double default_weight = 1, bool include_destructables = true);

		void update()
		{
			_setDefaultGrids();
		}

		::sc2::ImageData getDefaultGridData();

		// Given a point, flood fill outward from it and return the valid points.
		// Does not continue through chokes.
		std::vector<::sc2::Point2D> GetFloodFillArea(::sc2::Point2D start_point, int max_distance);

	private:
		AeolusBot& m_bot;
		const::sc2::ObservationInterface* m_observation;
		Grid m_pathing_grid;
		Grid m_placing_grid;
		::sc2::HeightMap m_terrain_map;
		Grid m_default_grid;
		Grid m_default_grid_cleaned;

		std::map<std::tuple<float, float>, const ::sc2::Unit*> m_destructables_included;
		std::map<std::tuple<float, float>, const ::sc2::Unit*> m_minerals_included;

		void _setDefaultGrids();
		Grid _addNonPathablesGround(Grid base_grid, bool include_destructables = true);
	};
}