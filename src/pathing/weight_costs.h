#pragma once
#include <sc2api/sc2_unit.h>
#include <map>

namespace Aeolus {
	namespace constants
	{
		struct UnitWeight
		{
			double AirCost;
			double GroundCost;
			double AirRange;
			double GroundRange;
		};

        std::map<::sc2::UnitTypeID, UnitWeight> WEIGHT_COSTS =
        {
            {::sc2::UNIT_TYPEID::PROTOSS_ADEPT, {0, 9, 0, 5}},
            {::sc2::UNIT_TYPEID::PROTOSS_ADEPTPHASESHIFT, {0, 9, 0, 5}},
            {::sc2::UNIT_TYPEID::TERRAN_AUTOTURRET, {31, 31, 7, 7}},
            {::sc2::UNIT_TYPEID::PROTOSS_ARCHON, {40, 40, 3, 3}},
            {::sc2::UNIT_TYPEID::ZERG_BANELING, {0, 20, 0, 3}},
            {::sc2::UNIT_TYPEID::TERRAN_BANSHEE, {0, 12, 0, 6}},
            {::sc2::UNIT_TYPEID::TERRAN_BATTLECRUISER, {31, 50, 6, 6}},
            {::sc2::UNIT_TYPEID::PROTOSS_CARRIER, {20, 20, 11, 11}},
            {::sc2::UNIT_TYPEID::ZERG_CORRUPTOR, {10, 0, 6, 0}},
            {::sc2::UNIT_TYPEID::TERRAN_CYCLONE, {27, 27, 7, 7}},
            {::sc2::UNIT_TYPEID::TERRAN_GHOST, {10, 10, 6, 6}},
            {::sc2::UNIT_TYPEID::TERRAN_HELLION, {0, 8, 0, 8}},
            {::sc2::UNIT_TYPEID::ZERG_HYDRALISK, {20, 20, 6, 6}},
            {::sc2::UNIT_TYPEID::ZERG_INFESTOR, {30, 30, 10, 10}},
            {::sc2::UNIT_TYPEID::TERRAN_LIBERATOR, {10, 0, 5, 0}},
            {::sc2::UNIT_TYPEID::TERRAN_MARINE, {10, 10, 5, 5}},
            {::sc2::UNIT_TYPEID::PROTOSS_MOTHERSHIP, {23, 23, 7, 7}},
            {::sc2::UNIT_TYPEID::ZERG_MUTALISK, {8, 8, 3, 3}},
            {::sc2::UNIT_TYPEID::PROTOSS_ORACLE, {0, 24, 0, 4}},
            {::sc2::UNIT_TYPEID::PROTOSS_PHOENIX, {15, 0, 7, 0}},
            {::sc2::UNIT_TYPEID::ZERG_QUEEN, {12.6, 11.2, 7, 5}},
            {::sc2::UNIT_TYPEID::PROTOSS_SENTRY, {8.4, 8.4, 5, 5}},
            {::sc2::UNIT_TYPEID::PROTOSS_STALKER, {10, 10, 6, 6}},
            {::sc2::UNIT_TYPEID::PROTOSS_TEMPEST, {17, 17, 14, 10}},
            {::sc2::UNIT_TYPEID::TERRAN_THOR, {28, 28, 11, 7}},
            {::sc2::UNIT_TYPEID::TERRAN_VIKINGASSAULT, {0, 17, 0, 6}},
            {::sc2::UNIT_TYPEID::TERRAN_VIKINGFIGHTER, {14, 0, 9, 0}},
            {::sc2::UNIT_TYPEID::PROTOSS_VOIDRAY, {20, 20, 6, 6}},
            {::sc2::UNIT_TYPEID::TERRAN_WIDOWMINEBURROWED, {150, 150, 5.5, 5.5}}
        };
	}
}