#pragma once

#include "grid.h"
#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_interfaces.h>

#include <Eigen/Dense>
#include <map>

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

	private:
		AeolusBot& m_bot;
		const::sc2::ObservationInterface* m_observation;
		Grid m_pathing_grid;
		Grid m_placing_grid;
		Grid m_terrain_map;
		Grid m_default_grid;
		Grid m_default_grid_cleaned;

		std::map<::sc2::Point2D, const ::sc2::Unit*> m_destructables_included;
		std::map<::sc2::Point2D, const ::sc2::Unit*> m_minerals_included;

		void _setDefaultGrids();
		Grid _addNonPathablesGround(Grid base_grid, bool include_destructables = true);
	};
}