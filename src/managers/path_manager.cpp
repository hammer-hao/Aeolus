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
			m_cached_ground_grid = m_ground_grid;
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

	void PathManager::_addUnitInfluence(::sc2::Unit* unit)
	{
		if (constants::WEIGHT_COSTS.find(unit->unit_type)
			!= constants::WEIGHT_COSTS.end())
		{
			// if we pre-defined unit ground/air weight and range

		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_DISRUPTORPHASED)
		{
			// A disruptor Nova
		}
		else if (unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_BANELING)
		{
			// A baneling
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
			if (ManagerMediator::getInstance().GroundRange(m_bot, unit) < 2)
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

	::sc2::ImageData PathManager::_getDefaultGridData()
	{
		return m_mapdata.getDefaultGridData();
	}
}