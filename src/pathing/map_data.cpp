#include "map_data.h"
#include "Eigen/Dense"
#include "../utils/grid_utils.h"
#include "../utils/unit_utils.h"
#include "../Aeolus.h"
#include "../managers/manager_mediator.h"

namespace Aeolus
{
	MapData::MapData(AeolusBot& aeolusbot)
		: m_bot(aeolusbot), m_observation(aeolusbot.Observation()),
		m_pathing_grid(m_observation->GetGameInfo().pathing_grid),
		m_placing_grid(m_observation->GetGameInfo().placement_grid),
		m_terrain_map(m_observation->GetGameInfo().terrain_height)
	{
	}

	void MapData::_setDefaultGrids()
	{
		m_default_grid = Grid(m_pathing_grid.GetGrid().cwiseMax(m_placing_grid.GetGrid()));
		// m_default_grid.FlipValues();

		::sc2::Units destructables = ManagerMediator::getInstance().GetAllDestructables(m_bot);

		int pathable = 1, non_pathable = 0;

		for (auto& destructable : destructables)
		{
			// cleaning all destructable rocks
			m_destructables_included[utils::ConvertTo2D(destructable->pos)] = destructable;
			utils::SetDestructableStatus(m_default_grid, destructable, pathable);
		}

		for (auto& mineral : ManagerMediator::getInstance().GetAllMineralPatches(m_bot))
		{
			// cleaning all mineral fields
			m_minerals_included[utils::ConvertTo2D(mineral->pos)] = mineral;
			int x_1 = static_cast<int>(mineral->pos.x);
			int x_2 = x_1 - 1;
			int y = static_cast<int>(mineral->pos.y);
			m_default_grid.SetValue(x_1, y, non_pathable);
			m_default_grid.SetValue(x_2, y, non_pathable);
		}

		for (auto& geyser : ManagerMediator::getInstance().GetAllVespeneGeysers(m_bot))
		{
			int width = 3;
			int height = 3;
			// clearning all vespene geysers
			int x_start = static_cast<int>(geyser->pos.x - 1.5);
			int y_start = static_cast<int>(geyser->pos.y - 1.5);
			m_default_grid.SetBlockValue(x_start, y_start, width, height, non_pathable);
		}


		// now that we cleaned all destructables, minerals, and geysers
		// make a copy of this clearned grid to keep
		m_default_grid_cleaned = m_default_grid;
	}

	Grid MapData::GetAStarGrid(double default_weight, bool include_destructables)
	{
		return _addNonPathablesGround(m_default_grid, true);
	}

	Grid MapData::_addNonPathablesGround(Grid base_grid, bool include_destructables)
	{
		int pathable = 1, non_pathable = 0;
		::sc2::Units structures = ManagerMediator::getInstance().GetAllStructures(m_bot, ::sc2::Unit::Alliance::Self);
		for (auto& structure : structures)
		{
			int size;
			if (constants::BUILDINGS_2X2.find(structure->unit_type) != constants::BUILDINGS_2X2.end())
				size = 2;
			else if (constants::BUILDINGS_3X3.find(structure->unit_type) != constants::BUILDINGS_3X3.end())
				size = 3;
			else if (constants::BUILDING_5X5.find(structure->unit_type) != constants::BUILDING_5X5.end())
				size = 5;
			// left bottom point
			int x_start = static_cast<int>(structure->pos.x - (size / 2));
			int y_start = static_cast<int>(structure->pos.y - (size / 2));
			int x_end = static_cast<int>(x_start + size);
			int y_end = static_cast<int>(y_start + size);
			base_grid.SetBlockValue(x_start, y_start, size, size, non_pathable);

			if (size == 5)
			{
				base_grid.SetValue(x_start, y_start, pathable);
				base_grid.SetValue(x_start, y_end - 1, pathable);
				base_grid.SetValue(x_end - 1, y_start, pathable);
				base_grid.SetValue(x_end - 1, y_end - 1, pathable);
			}
		}

		if (m_minerals_included.size() != ManagerMediator::getInstance().GetAllMineralPatches(m_bot).size())
		{
			std::stringstream debugMessage;
			debugMessage << "Detected Mineral change!";
			m_bot.Actions()->SendChat(debugMessage.str());
			std::set<::sc2::Point2D> current_minerals;
			for (auto& mineral : ManagerMediator::getInstance().GetAllMineralPatches(m_bot))
			{
				current_minerals.insert(utils::ConvertTo2D(mineral->pos));
			}
			auto it = m_minerals_included.begin();
			while (it != m_minerals_included.end())
			{
				if (current_minerals.find(it->first) == current_minerals.end())
				{
					int x_1 = static_cast<int>(it->first.x);
					int x_2 = x_1 - 1;
					int y = static_cast<int>(it->first.y);

					base_grid.SetValue(x_1, y, pathable);
					base_grid.SetValue(x_2, y, pathable);

					m_default_grid.SetValue(x_1, y, pathable);
					m_default_grid.SetValue(x_2, y, pathable);

					// Remove the mineral from m_minerals_included.
					it = m_minerals_included.erase(it);
				}
				else ++it;
			}
		}

		return base_grid;
	}

	::sc2::ImageData MapData::getDefaultGridData()
	{
		// return utils::To8BPPImageData(m_default_grid, 1);
		return utils::To8BPPImageData(GetAStarGrid(), 1);
	}
}