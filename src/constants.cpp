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
			case ManagerName::TARGET_MANAGER: return "TargetManager";
			case ManagerName::NEUTRAL_UNIT_MANAGER: return "NeutralUnitManager";
			case ManagerName::UNIT_FILTER_MANAGER: return "UnitFilterManager";
			case ManagerName::UNIT_PROPERTY_MANAGER: return "UnitPropertyManager";
			case ManagerName::DEFENSE_MANAGER: return "DefenseManager";
			case ManagerName::DOOR_MANAGER: return "DoorManager";
			case ManagerName::PLACEMENT_MANAGER: return "PlacementManager";
			case ManagerName::BUILDING_MANAGER: return "BuildingManager";
			case ManagerName::BUDGET_MANAGER: return "BudgetManager";
			case ManagerName::COMBAT_SIM_MANAGER: return "CombatSimManager";
			case ManagerName::ARMY_COMPOSITION_MANAGER: return "ArmyCompositionManager";
			case ManagerName::SCOUTING_MANAGER: return "ScoutingManager";
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

		const ::sc2::UNIT_TYPEID isResearchedFrom(::sc2::UPGRADE_ID upgrade_id)
		{
			switch (upgrade_id)
			{
			case (::sc2::UPGRADE_ID::BLINKTECH): return ::sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL;
			case (::sc2::UPGRADE_ID::CHARGE): return ::sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL;
			case (::sc2::UPGRADE_ID::DARKTEMPLARBLINKUPGRADE): return ::sc2::UNIT_TYPEID::PROTOSS_DARKSHRINE;
			case (::sc2::UPGRADE_ID::EXTENDEDTHERMALLANCE): return ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
			case (::sc2::UPGRADE_ID::GRAVITICDRIVE): return ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
			case (::sc2::UPGRADE_ID::OBSERVERGRAVITICBOOSTER): return ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
			case (::sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL1): return ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
			case (::sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL2): return ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
			case (::sc2::UPGRADE_ID::PROTOSSAIRARMORSLEVEL3): return ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
			case (::sc2::UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL1): return ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
			case (::sc2::UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2): return ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
			case (::sc2::UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL3): return ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
			case (::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL1): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL2): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL3): return ::sc2::UNIT_TYPEID::PROTOSS_FORGE;
			case (::sc2::UPGRADE_ID::PSISTORMTECH): return ::sc2::UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE;
			case (::sc2::UPGRADE_ID::VOIDRAYSPEEDUPGRADE): return ::sc2::UNIT_TYPEID::PROTOSS_FLEETBEACON;
			case (::sc2::UPGRADE_ID::WARPGATERESEARCH): return ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
			}
		}
    }
}