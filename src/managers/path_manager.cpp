#include "path_manager.h"
#include "../pathing/weight_costs.h"
#include "../Aeolus.h"
#include "../constants.h"
#include "manager_mediator.h"

namespace Aeolus
{
	void PathManager::update(int iteration)
	{
		if (iteration == 0)
		{
			m_mapdata.update();
			m_ground_grid = m_mapdata.GetAStarGrid();
		}
		else if (iteration > 0)
		{
			_reset_grids(); // clean the grids before populating

			::sc2::Units enemy_units = ManagerMediator::getInstance().GetAllEnemyUnits(m_bot);

			for (const auto unit : enemy_units)
			{
				AddUnitInfluence(unit);
			}
		}
	}

	std::any PathManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_DEFAULT_GRID_DATA):
			{
				return _getDefaultGridData();
			}
		default:
			return 0;
		}
	}

	void PathManager::AddUnitInfluence(const ::sc2::Unit* enemy)
	{
		_addUnitInfluence(enemy);
	}

	void PathManager::_addUnitInfluence(const ::sc2::Unit* unit)
	{
		if (constants::WEIGHT_COSTS.find(unit->unit_type)
			!= constants::WEIGHT_COSTS.end())
		{
			// if we pre-defined unit ground/air weight and range
			auto it = constants::WEIGHT_COSTS.find(unit->unit_type);
			double ground_cost = it->second.GroundCost;
			double ground_range = it->second.GroundRange;

			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer, ground_cost);
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED)
		{
			// A disruptor Nova
			double ground_cost = 1000;
			double ground_range = 8 + Config::range_buffer;
			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range, ground_cost);

		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_BANELING)
		{
			// A baneling
			// this should already by in the weight_cost dict! monitor if we need to add more logic!
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_INFESTOR && unit->energy >= 75)
		{
			// infestor with fungal
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ORACLE && unit->energy >= 25)
		{
			// oracle with pulsar beam
		}
		
		else if (ManagerMediator::getInstance().CanAttackGround(m_bot, unit))
		{
			double ground_range = ManagerMediator::getInstance().GroundRange(m_bot, unit);
			double ground_dps = ManagerMediator::getInstance().GroundDPS(m_bot, unit);
			m_ground_grid.AddCost(unit->pos.x, unit->pos.y, ground_range + Config::range_buffer, ground_dps);

			if (ground_range < 2)
			{
				// melee units
			}
			else
			{
				// non-melee units
				// handle ground attack here

				if (ManagerMediator::getInstance().CanAttackAir(m_bot, unit))
				{
					// handle air attack
				}
			}
		}
		else if (ManagerMediator::getInstance().CanAttackAir(m_bot, unit))
		{
			// units with air attack only (no attack vs ground)
			// TODO: if unit is flying, get ground to air grid and air to air grid 
		}
	}

	void PathManager::_reset_grids()
	{
		m_ground_grid.Reset();
	}

	::sc2::ImageData PathManager::_getDefaultGridData()
	{
		return m_mapdata.getDefaultGridData();
	}
}