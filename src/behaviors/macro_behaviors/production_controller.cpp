#include "production_controller.h"
#include "build_structure.h"

#include "../../managers/manager_mediator.h"
#include "macro_behavior.h"
#include "../../Aeolus.h"
#include "../../constants.h"
#include "../../utils/unit_utils.h"
#include <map>
#include <optional>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_score.h>

namespace Aeolus
{
	bool ProductionController::execute(AeolusBot& aeolusbot)
	{
		auto& mediator = ManagerMediator::getInstance();
		auto* observation = aeolusbot.Observation();
		float mineral_collection_rate = observation->GetScore().score_details.collection_rate_minerals;
		float gas_collection_rate = observation->GetScore().score_details.collection_rate_vespene;
		int minerals = observation->GetMinerals();
		int vespene = observation->GetVespene();
		::sc2::Units all_own_units = mediator.GetAllOwnUnits(aeolusbot);
		::sc2::Units all_own_structures = mediator.GetAllOwnStructures(aeolusbot);

		std::map<::sc2::UNIT_TYPEID, size_t> unit_count_map;
		size_t total_unit_count = 0;

		for (const auto& unit : all_own_units)
			if (m_army_composition_map.find(unit->unit_type) != m_army_composition_map.end())
			{
				unit_count_map[unit->unit_type]++;
				total_unit_count++;
			}

		for (const auto& item : m_army_composition_map)
		{
			::sc2::UNIT_TYPEID unit_type = item.first;
			float proportion = item.second;

			int num_units = 0;
			for (const auto& unit : all_own_units) if (unit->unit_type == unit_type) ++num_units;

			float current_proportion = static_cast<float>(unit_count_map[unit_type]) / static_cast<float>(total_unit_count);

			auto trained_from = utils::_isTrainedFrom(unit_type);
			if (trained_from.has_value())
			{
				if (_techUp(aeolusbot, unit_type)) return true;

				if (minerals > m_add_production_at_bank.first
					&& vespene > m_add_production_at_bank.second)
				{
					size_t existing_production_count = 0;

					for (const auto& structure : all_own_structures)
						if (structure->unit_type == trained_from) existing_production_count++;
					existing_production_count += mediator.GetNumberPending(aeolusbot, trained_from.value());

					_buildProductionDueToBank(aeolusbot, unit_type, mineral_collection_rate, gas_collection_rate,
						existing_production_count, trained_from.value(), proportion);
				}
			}
		}
		return true;
	}

	bool ProductionController::_techUp(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID unit_type, int base_location)
	{
		return false;
	}

	bool ProductionController::_buildProductionDueToBank(AeolusBot& aeolusbot,
		::sc2::UNIT_TYPEID unit_type,
		float mineral_collection_rate,
		float gas_collection_rate,
		size_t existing_production_count,
		::sc2::UNIT_TYPEID production_structure_id,
		float target_proportion)
	{
		std::cout << "Our current mineral income is: " << mineral_collection_rate << std::endl;
		std::cout << "Our current gas income is:" << gas_collection_rate << std::endl;

		auto& mediator = ManagerMediator::getInstance();
		auto unit_cost = mediator.GetUnitCost(aeolusbot, unit_type);

		int rate_supported_by_minerals = static_cast<int>(
			mineral_collection_rate / (unit_cost.first + 1)
			* m_alpha
			* target_proportion
			);

		int rate_supported_by_gas = static_cast<int>(
			gas_collection_rate / (unit_cost.second + 1)
			* m_alpha
			* target_proportion
			);

		/*std::cout << "We can currently support " << std::min(rate_supported_by_gas, rate_supported_by_minerals)
			<< " simutaneous productions." << std::endl;
		std::cout << "Existing production count: " << existing_production_count << std::endl;*/

		if (existing_production_count < rate_supported_by_minerals
			&& existing_production_count < rate_supported_by_gas)
		{
			return std::make_unique<BuildStructure>(production_structure_id, m_base_location, false).get()->execute(aeolusbot);
			// std::cout << "Building more production to match our income... " << std::endl;
		}
		return false;
	}
}