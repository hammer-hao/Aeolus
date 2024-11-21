#include "map_data.h"
#include "../managers/manager_mediator.h"

namespace Aeolus
{
	void MapData::_setDefaultGrids()
	{
		m_default_grid = Grid(m_pathing_grid.GetGrid().cwiseMax(m_placing_grid.GetGrid()));
		m_default_grid_cleaned = m_default_grid;

		std::cout << "can we get the destructables?" << std::endl;
		::sc2::Units destructables = ManagerMediator::getInstance().GetAllDestructables(m_bot);
		std::cout << "yes we can!" << std::endl;
	}

	Grid MapData::GetAStarGrid(double default_weight, bool include_destructables)
	{
		return m_default_grid;
	}
}