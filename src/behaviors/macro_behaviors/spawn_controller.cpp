#include "spawn_controller.h"

#include "macro_behavior.h"

#include <sc2api/sc2_typeenums.h>
#include <sc2api/sc2_common.h>
#include <map>
#include <vector>
#include <optional>

#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../utils/unit_utils.h"

namespace Aeolus
{
	bool SpawnController::execute(AeolusBot& aeolusbot)
	{
		auto* observation = aeolusbot.Observation();
		auto& mediator = ManagerMediator::getInstance();
		std::vector<std::pair<::sc2::UNIT_TYPEID, ::sc2::UNIT_TYPEID>> tech_ready_for;

		for (const auto& spawn_type : m_army_composition_map)
		{
			::sc2::UNIT_TYPEID unit_type = spawn_type.first;

			::sc2::UNIT_TYPEID required_tech = mediator.GetRequiredTech(aeolusbot, unit_type);
			
			for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
			{
				if (structure->unit_type == required_tech && structure->build_progress >= 1.0f)
				{
					auto prod = utils::_isTrainedFrom(unit_type).value();
					tech_ready_for.push_back({ unit_type, prod });
					break;
				}
			}
		}

		// If we can only build one type of unit:
		if (observation->GetGameLoop() % 200 == 50)
			std::cout << "tech ready for: " << tech_ready_for.size() << std::endl;

		if (tech_ready_for.size() == 1)
		{
			::sc2::Units production_structures = _getBuildStructures(aeolusbot,
				tech_ready_for[0].second,
				tech_ready_for[0].first
			);

			if (observation->GetGameLoop() % 200 == 50)
				std::cout << "Number of productions ready: " << production_structures.size() << std::endl;

			int supply_left = observation->GetFoodCap() - observation->GetFoodUsed();

			int mineral_cost = 0;
			int vespene_cost = 0;
			int supply_cost = 0;

			int spawn_amount = _calculateBuildAmount(
				aeolusbot,
				tech_ready_for[0].first,
				production_structures,
				supply_left,
				20,
				supply_cost,
				mineral_cost,
				vespene_cost
			);

			while (spawn_amount > 0)
			{
				if (production_structures.empty()) break;
			 	m_production_to_unit_map[production_structures.back()] = tech_ready_for[0].first;
				production_structures.pop_back();
			}
		}

		return _spawnUnits(aeolusbot);
	}

	::sc2::Units SpawnController::_getBuildStructures(AeolusBot& aeolusbot,
			::sc2::UNIT_TYPEID structure_type, ::sc2::UNIT_TYPEID spawn_type)
	{
		::sc2::Units result;
		::sc2::Units can_build_from;

		for (const auto& structure : ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot))
		{
			if (structure->unit_type == structure_type) can_build_from.push_back(structure);
		}
		// std::cout << "build structures has " << can_build_from.size() << " structures." << std::endl;

		if (can_build_from.empty()) return {};

		if (structure_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPGATE)
		{
			::sc2::Units can_warp_in_from;
			for (const auto& warpgate : can_build_from)
			{
				auto abilities = aeolusbot.Query()->GetAbilitiesForUnit(warpgate);
				for (const auto& ability : abilities.abilities)
				{
					if (ability.ability_id == ::sc2::ABILITY_ID::TRAINWARP_ZEALOT)
					{
						can_warp_in_from.push_back(warpgate);
						break;
					}
				}
			}
			can_build_from = can_warp_in_from;
		}

		for (const auto& production : can_build_from)
		{
			if (m_production_to_unit_map.find(production) == m_production_to_unit_map.end())
			{
				if ((production->is_powered || production->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_NEXUS)
					&& production->orders.size() == 0
					&& production->build_progress >= 1.0f)
				{
					result.push_back(production);
				}
			}
		}
		return result;
	}

	int SpawnController::_calculateBuildAmount(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID spawn_type,
		const ::sc2::Units& production_structures, int supply_left, int limit,
		int& supply_cost, int& mineral_cost, int& vespene_cost)
	{
		int minerals = aeolusbot.Observation()->GetMinerals();
		int gas = aeolusbot.Observation()->GetVespene();

		auto unit_cost = ManagerMediator::getInstance().GetUnitCost(aeolusbot, spawn_type);
		mineral_cost = unit_cost.first;
		vespene_cost = unit_cost.second;
		supply_cost = ManagerMediator::getInstance().GetUnitSupplyCost(aeolusbot, spawn_type);

		int amount = std::min({
			static_cast<int>(production_structures.size()),
			limit,
			supply_left / supply_cost,
			mineral_cost > 1? minerals / mineral_cost : 999999,
			vespene_cost > 1 ? gas / vespene_cost : 999999
			}
		);

		return amount;
	}

	bool SpawnController::_spawnUnits(AeolusBot& aeolusbot)
	{
		bool executed = false;
		for (const auto& item : m_production_to_unit_map)
		{
			executed = true;
			if (item.first->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPGATE)
			{
				// warp in logic
			}
			else
			{
				auto spawn_ability = ManagerMediator::getInstance().GetCreationAbility(aeolusbot, item.second);
				aeolusbot.Actions()->UnitCommand(item.first, spawn_ability);
			}
		}
		return executed;
	}
}