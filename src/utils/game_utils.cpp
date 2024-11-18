#pragma once
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>

namespace Aeolus
{
    namespace utils
    {
        ::sc2::Units GetOwnedTownHalls(const sc2::ObservationInterface* observation) {
            return observation->GetUnits(sc2::Unit::Alliance::Self, [](const sc2::Unit& unit) {
                switch (unit.unit_type.ToType()) {
                case sc2::UNIT_TYPEID::TERRAN_COMMANDCENTER:
                case sc2::UNIT_TYPEID::TERRAN_ORBITALCOMMAND:
                case sc2::UNIT_TYPEID::TERRAN_PLANETARYFORTRESS:
                case sc2::UNIT_TYPEID::PROTOSS_NEXUS:
                case sc2::UNIT_TYPEID::ZERG_HATCHERY:
                case sc2::UNIT_TYPEID::ZERG_LAIR:
                case sc2::UNIT_TYPEID::ZERG_HIVE:
                    return true;
                default:
                    return false;
                }
                });
        }

        bool IsWorkerCarryingResource(const sc2::Unit* worker) {
            // Iterate through all buffs on the worker to see if any buff indicates resource carrying
            for (const auto& buff : worker->buffs) {
                if (buff == sc2::BUFF_ID::CARRYMINERALFIELDMINERALS ||
                    buff == sc2::BUFF_ID::CARRYHIGHYIELDMINERALFIELDMINERALS ||
                    buff == sc2::BUFF_ID::CARRYHARVESTABLEVESPENEGEYSERGAS) {
                    return true;  // Worker is carrying a resource
                }
            }
            return false;  // Worker is not carrying a resource
        }
    }
}
