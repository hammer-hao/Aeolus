#pragma once

#include "macro_behavior.h"

#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <map>
#include <unordered_map>
#include <optional>

namespace Aeolus
{
	class AeolusBot;
	class SpawnController : public MacroBehavior
	{
	public:
		SpawnController(std::map<::sc2::UNIT_TYPEID, float> army_composition_map,
			std::optional<::sc2::Point2D> spawn_target = std::nullopt) :
			m_army_composition_map(army_composition_map),
			m_spawn_target(spawn_target)
		{
		}
		~SpawnController() override = default;

		bool execute(AeolusBot& aeolusbot) override;

	private:
		std::map<::sc2::UNIT_TYPEID, float> m_army_composition_map;
		std::optional<::sc2::Point2D> m_spawn_target;
		std::unordered_map<const ::sc2::Unit*, ::sc2::UNIT_TYPEID> m_production_to_unit_map;

		/**
		* @brief Get ALL structures that can currently issue a build command for the
		* given unit type.
		*/
		::sc2::Units _getBuildStructures(AeolusBot& aeolusbot,
			::sc2::UNIT_TYPEID structure_type, ::sc2::UNIT_TYPEID spawn_type);

		static int _calculateBuildAmount(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID spawn_type,
			const ::sc2::Units& production_structures, int supply_left, int limit,
			int& supply_cost, int& mineral_cost, int& vespene_cost);

		bool _spawnUnits(AeolusBot& aeolusbot);
	};
}