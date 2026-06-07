#pragma once

#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_data.h>
#include "manager.h"
#include "../constants.h"

namespace Aeolus
{
	class AeousBot;
	class UnitFilterManager : public Manager
	{
	public:
		UnitFilterManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
			std::cout << "Unit filter manager initialization" << std::endl;
		}

		std::string_view GetName() const override {
			static const std::string name = "UnitFilterManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		// Implement the update method.
		void update(int iteration) override;

		void OnUnitCreated(const ::sc2::Unit* unit);

		void OnUnitDestroyed(const ::sc2::Unit* unit);

	private:
		::sc2::Units _getAllStructures(AeolusBot& aeolusbot, ::sc2::Unit::Alliance alliance);
		AeolusBot& m_bot;

		bool isStructurePresent(::sc2::UNIT_TYPEID structureType);

		// All unit data in the game. Cached at iteration 0 every game.
		::sc2::UnitTypes m_unit_data_cache;

		// our own units cache. these get updated at every iteration.
		::sc2::Units m_all_units;
		::sc2::Units m_all_structures;
		::sc2::Units m_watch_towers;
		::sc2::Units m_mineral_fields;
		::sc2::Units m_vespene_geysers;
		::sc2::Units m_resources;
		::sc2::Units m_destructables;
		::sc2::Units m_own_units;
		::sc2::Units m_own_structures;
		::sc2::Units m_own_townhalls;
		::sc2::Units m_own_ready_townhalls;
		::sc2::Units m_own_gas_buildings;
		::sc2::Units m_own_workers;
		::sc2::Units m_own_non_workers;
		::sc2::Units m_all_own_units;
		::sc2::Units m_gas_buildings;
		::sc2::Units m_enemy_units;
		::sc2::Units m_enemy_structures;
		::sc2::Units m_all_enemy_units;
		::sc2::Units m_enemy_town_halls;

		// Map to keep track of the known enemy units that are still alive
		std::unordered_map<::sc2::Tag, ::sc2::UNIT_TYPEID> m_knownEnemyUnits;

		// Map to keep track of the known adept shades
		std::unordered_map<::sc2::Tag, std::pair<::sc2::Tag, int>> m_adeptShadeTracker;
	};
}