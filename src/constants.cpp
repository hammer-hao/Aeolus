#include "constants.h"
#include <iostream>

namespace Aeolus {
    namespace constants 
	{
		std::string ManagerNameToString(ManagerName managername)
		{
			switch (managername)
			{
			case ManagerName::UNIT_ROLE_MANAGER: return "class Aeolus::UnitRoleManager";
			case ManagerName::RESOURCE_MANAGER: return "class Aeolus::ResourceManager";
			case ManagerName::PATH_MANAGER: return "class Aeolus::PathManager";
			case ManagerName::NEUTRAL_UNIT_MANAGER: return "class Aeolus::NeutralUnitManager";
			case ManagerName::UNIT_FILTER_MANAGER: return "class Aeolus::UnitFilterManager";
			case ManagerName::UNIT_PROPERTY_MANAGER: return "class Aeolus::UnitPropertyManager";
			default: throw std::invalid_argument("Unknown Manager Name");
			}
		}

		std::string ManagerRequestTypeToString(ManagerRequestType request)
		{
			switch (request)
			{
			case ManagerRequestType::GET_UNITS_FROM_ROLE: return "GetUnitsFromRole";
			default: throw std::invalid_argument("Unknown Request Type");
			}
		}
    }
}