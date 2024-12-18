#include "constants.h"
#include <iostream>

namespace Aeolus {
    namespace constants 
	{
		std::string ManagerNameToString(ManagerName managername)
		{
			switch (managername)
			{
			case ManagerName::UNIT_ROLE_MANAGER: return "UnitRoleManager";
			case ManagerName::RESOURCE_MANAGER: return "ResourceManager";
			case ManagerName::PATH_MANAGER: return "PathManager";
			case ManagerName::NEUTRAL_UNIT_MANAGER: return "NeutralUnitManager";
			case ManagerName::UNIT_FILTER_MANAGER: return "UnitFilterManager";
			case ManagerName::UNIT_PROPERTY_MANAGER: return "UnitPropertyManager";
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