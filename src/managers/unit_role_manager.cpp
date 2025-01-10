#include "unit_role_manager.h"
#include "../constants.h"
#include <iostream>
#include <vector>
#include <sc2api/sc2_common.h>
#include <tuple>

namespace Aeolus {

	std::any UnitRoleManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{

		// std::cout << "UnitRoleManager: Received request" << std::endl;

		switch (request)
		{
		case constants::ManagerRequestType::GET_UNITS_FROM_ROLE:
		{
			// std::cout << "UnitRoleManager: Request Dispatched to GetUnitsFromRole" << std::endl;
			auto params = std::any_cast<std::tuple<constants::UnitRole>>(args);
			constants::UnitRole role = std::get<0>(params);
			return GetUnitsFromRole(role);
		}
		case constants::ManagerRequestType::ASSIGN_ROLE:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*, constants::UnitRole>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			constants::UnitRole unit_role = std::get<1>(params);
			AssignRole(unit, unit_role);
			return 0;
		}
		case constants::ManagerRequestType::CATCH_UNIT:
		{
			// std::cout << "UnitRoleManager: Request Dispatched to CatchUnit" << std::endl;

			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			CatchUnit(unit);
			return 0;
		}
		default:

			throw std::runtime_error("Unhandled request type in ProcessRequest");
		}
	}

	void UnitRoleManager::ClearRole(const ::sc2::Unit* unit)
		// clear the role of a unit
	{
		for (auto& [role, units]: unit_role_map_)
		{
			auto it = std::find(units.begin(), units.end(), unit);

			// if the unit is found, erase it and return early
			if (it != units.end())
			{
				units.erase(it);
				m_assigned_units.erase(unit);
				//std::cout << "Unit with tag" << unit->tag << "Cleared from role "
				//	<< static_cast<int>(role) << std::endl;
				return;
			}
		}
	}
		
	void UnitRoleManager::AssignRole(const ::sc2::Unit* unit, constants::UnitRole unitrole)
	{
		// Assign a new role to a unit, removes the previous role from the unit
		
		ClearRole(unit);
		unit_role_map_[unitrole].push_back(unit);
		m_assigned_units.insert(unit);
		// std::cout << "Assigned role to unit of type: " << unit->unit_type << std::endl;
	}

	void UnitRoleManager::CatchUnit(const ::sc2::Unit* unit)
	{
		// std::cout << "UnitRoleManager: Catching Unit... " << std::endl;
		if (m_assigned_units.find(unit) != m_assigned_units.end()) return;
		if (unit->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PROBE ||
			unit->unit_type == ::sc2::UNIT_TYPEID::TERRAN_SCV ||
			unit->unit_type == ::sc2::UNIT_TYPEID::ZERG_DRONE)
		{
			AssignRole(unit, constants::UnitRole::GATHERING);
		}
	}

	::sc2::Units UnitRoleManager::GetUnitsFromRole(constants::UnitRole role) const
	{
		// std::cout << "UnitRoleManager: Finding units..." << std::endl;

		auto it = unit_role_map_.find(role);
		if (it != unit_role_map_.end())
		{
			// std::cout << "UnitRoleManager: Found units!" << std::endl;
			return ::sc2::Units(it->second);
		}

		return {};
	}
}