#pragma once

#include "grid.h"
#include "../Aeolus.h"
#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_interfaces.h>

#include <Eigen/Dense>

namespace Aeolus
{

	class MapData
	{
	public:
		MapData(AeolusBot& aeolusbot)
			: m_bot(aeolusbot), m_observation(aeolusbot.Observation()),
			m_pathing_grid(m_observation->GetGameInfo().pathing_grid),
			m_placing_grid(m_observation->GetGameInfo().placement_grid),
			m_terrain_map(m_observation->GetGameInfo().terrain_height)
		{
		}

		Grid GetAStarGrid(double default_weight = 1, bool include_destructables = true);

		void update()
		{
			_setDefaultGrids();
		}

	private:
		AeolusBot& m_bot;
		const::sc2::ObservationInterface* m_observation;
		Grid m_pathing_grid;
		Grid m_placing_grid;
		Grid m_terrain_map;
		Grid m_default_grid;
		Grid m_default_grid_cleaned;

		void _setDefaultGrids();
		Grid _addNonPathablesGround(Grid base_grid, bool include_destructables = true);
	};
}