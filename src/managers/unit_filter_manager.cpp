#include "unit_filter_manager.h"
#include "../Aeolus.h"
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_data.h>

namespace Aeolus
{
	std::any UnitFilterManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_ALL_ENEMY_UNITS):
		{
			return m_all_enemy_units;
		}
		case (constants::ManagerRequestType::GET_ALL_STRUCTURES):
		{	
			return m_all_structures;
		}
		case (constants::ManagerRequestType::GET_ALL_OWN_UNITS):
		{
			return m_all_own_units;
		}
		case (constants::ManagerRequestType::GET_OWN_WORKERS):
		{
			return m_own_workers;
		}
		case (constants::ManagerRequestType::GET_OWN_TOWN_HALLS):
		{
			return m_own_townhalls;
		}
		case (constants::ManagerRequestType::GET_ALL_MINERAL_PATCHES):
		{
			return m_mineral_fields;
		}
		case (constants::ManagerRequestType::GET_ALL_VESPENE_GEYSERS):
		{
			return m_vespene_geysers;
		}
		case (constants::ManagerRequestType::GET_ALL_DESTRUCTABLES):
		{
			return m_destructables;
		}
		default:
			return 0;
		}
	}

	void UnitFilterManager::update(int iteration)
	{
		// std::cout << "Update UnitFilterManager at iteration " << iteration << std::endl;
		if (iteration == 0) m_unit_data_cache = m_bot.Observation()->GetUnitTypeData();
		m_all_units.clear();
		m_all_structures.clear();
		m_watch_towers.clear();
		m_mineral_fields.clear();
		m_vespene_geysers.clear();
		m_resources.clear();
		m_destructables.clear();
		m_own_units.clear();
		m_own_structures.clear();
		m_own_townhalls.clear();
		m_own_workers.clear();
		m_all_own_units.clear();
		m_gas_buildings.clear();
		m_enemy_units.clear();
		m_enemy_structures.clear();
		m_all_enemy_units.clear();

		// std::cout << "Size of m_all_structures " << m_all_structures.size() << std::endl;

		::sc2::Units all_units = m_bot.Observation()->GetUnits();
		for (const auto unit : all_units)
		{
			// add every unit to m_all_units;
			m_all_units.push_back(unit);

			int64_t unit_type = unit->unit_type;
			::sc2::UnitTypeData unit_typedata = m_unit_data_cache[unit_type];

			switch (unit->alliance)
			{
			case (::sc2::Unit::Alliance::Neutral):
			{
				// Neutral units
				if (unit_type == static_cast<int>(::sc2::UNIT_TYPEID::NEUTRAL_XELNAGATOWER))
				{
					m_watch_towers.push_back(unit);
				}
				else if (constants::MINERAL_IDS.find(unit->unit_type) != constants::MINERAL_IDS.end())
				{
					m_mineral_fields.push_back(unit);
				}
				else if (constants::VESPENE_IDS.find(unit->unit_type) != constants::VESPENE_IDS.end())
				{
					m_vespene_geysers.push_back(unit);
				}
				else
				{
					m_destructables.push_back(unit);
				}
				break;
			}
			case (::sc2::Unit::Alliance::Self):
			{
				m_all_own_units.push_back(unit);
				if (std::find(unit_typedata.attributes.begin(), unit_typedata.attributes.end(), ::sc2::Attribute::Structure)
					!= unit_typedata.attributes.end())
				{
					m_all_structures.push_back(unit);
					m_own_structures.push_back(unit);
					if (unit_type == static_cast<int>(::sc2::UNIT_TYPEID::PROTOSS_NEXUS))
					{
						m_own_townhalls.push_back(unit);
					}
				}
				else
				{
					m_own_units.push_back(unit);
					if (constants::WORKER_TYPES.find(unit->unit_type) != constants::WORKER_TYPES.end())
					{
						m_own_workers.push_back(unit);
					}
				}
				break;
			}
			case (::sc2::Unit::Alliance::Enemy):
			{
				m_all_enemy_units.push_back(unit);
				if (std::find(unit_typedata.attributes.begin(), unit_typedata.attributes.end(), ::sc2::Attribute::Structure)
					!= unit_typedata.attributes.end())
				{
					m_all_structures.push_back(unit);
					m_enemy_structures.push_back(unit);
				}
				else
				{
					m_enemy_units.push_back(unit);
				}
				break;
			}
			default:
				continue;
			}

			// std::cout << "Size of m_all_structures " << m_all_structures.size() << std::endl;
		}
	}

	::sc2::Units UnitFilterManager::_getAllStructures(AeolusBot& aeolusbot, ::sc2::Unit::Alliance alliance)
	{
		const ::sc2::ObservationInterface* observation = aeolusbot.Observation();
		return observation->GetUnits(
			alliance, 
			[observation](const ::sc2::Unit& unit)
			{
				const ::sc2::UnitTypeData& unit_type_data = observation->GetUnitTypeData()[unit.unit_type];
				return constants::BUILDINGS.find(unit.unit_type) != constants::BUILDINGS.end();
				/*
				return (unit_type_data.movement_speed == 0.0f && // Structures don't move
				unit_type_data.build_time > 0.0f && // Structures take time to build
				unit_type_data.food_required == 0.0f); // Structures don't consume supply
				*/
			});
	}
}