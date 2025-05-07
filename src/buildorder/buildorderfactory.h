#pragma once

#include "buildorderenum.h"
#include "buildorder.h"
#include "buildorderstep.h"
#include "structurebuildorderstep.h"
#include "upgradebuildorderstep.h"
#include "unitbuildorderstep.h"
#include "chronobuildorderstep.h"
#include <stdexcept>

#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class BuildOrderFactory
	{
	public:
		inline static std::unique_ptr<BuildOrder> makeBuildOrder(AeolusBot& aeolusbot, BuildOrderEnum buildOrderEnum)
		{
			std::vector<std::unique_ptr<BuildOrderStep>> toAdd;
			if (buildOrderEnum == BuildOrderEnum::MACRO_STALKERS)
			{
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(13, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(16, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(16, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(18, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false));
				toAdd.push_back(std::make_unique<UpgradeBuildOrderStep>(20, ::sc2::UPGRADE_ID::WARPGATERESEARCH));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false, 1));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
			}
			else if (buildOrderEnum == BuildOrderEnum::STALKER_IMMORTAL)
			{
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(13, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(16, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(18, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, false));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<UpgradeBuildOrderStep>(20, ::sc2::UPGRADE_ID::WARPGATERESEARCH));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL));
			}
			else if (buildOrderEnum == BuildOrderEnum::BLINK_STALKERS)
			{
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(13, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(16, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(18, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false, 1));
				toAdd.push_back(std::make_unique<UpgradeBuildOrderStep>(20, ::sc2::UPGRADE_ID::WARPGATERESEARCH));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<UpgradeBuildOrderStep>(20, ::sc2::UPGRADE_ID::BLINKTECH));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
			}
			else if (buildOrderEnum == BuildOrderEnum::MASS_ROBO)
			{
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(13, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY, false));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSBAY, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false));
				toAdd.push_back(std::make_unique<UpgradeBuildOrderStep>(20, ::sc2::UPGRADE_ID::EXTENDEDTHERMALLANCE));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_COLOSSUS));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_COLOSSUS));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
			}
			else if (buildOrderEnum == BuildOrderEnum::STALKER_VOIDRAY)
			{
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(13, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(14, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(18, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, true, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(19, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STARGATE, true, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY, false, 1));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_STALKER));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_VOIDRAY));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, false));
			}
			else if (buildOrderEnum == BuildOrderEnum::FORGE_EXPAND)
			{
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(12, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(13, ::sc2::UNIT_TYPEID::PROTOSS_FORGE, true, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, false, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON, false, 1));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(15, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(16, ::sc2::UNIT_TYPEID::PROTOSS_CYBERNETICSCORE, true, 1));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(16, ::sc2::UNIT_TYPEID::PROTOSS_ZEALOT));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(17, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(18, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(18, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, false));
				toAdd.push_back(std::make_unique<UnitBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_ZEALOT));
				toAdd.push_back(std::make_unique<UpgradeBuildOrderStep>(20, ::sc2::UPGRADE_ID::WARPGATERESEARCH));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL, false));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_PYLON, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<StructureBuildOrderStep>(20, ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY, true));
				toAdd.push_back(std::make_unique<UpgradeBuildOrderStep>(20, ::sc2::UPGRADE_ID::BLINKTECH));

			}
			return std::make_unique<BuildOrder>(aeolusbot, std::move(toAdd));
		}
	};
}