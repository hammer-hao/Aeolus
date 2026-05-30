#include "production_controller.h"
#include "build_structure.h"

#include "../../managers/manager_mediator.h"
#include "macro_behavior.h"
#include "../../Aeolus.h"
#include "../../constants.h"
#include "../../utils/unit_utils.h"
#include "tech_up.h"
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
		int minerals = mediator.GetMinerals(aeolusbot);
		int vespene = mediator.GetVespene(aeolusbot);
		::sc2::Units all_own_units = mediator.GetAllOwnUnits(aeolusbot);
		::sc2::Units all_own_structures = mediator.GetAllOwnStructures(aeolusbot);

		std::map<::sc2::UNIT_TYPEID, size_t> unit_count_map;
		size_t total_unit_count = 0;

		for (const auto& unit : all_own_units)
		{
			if (m_army_composition_map.find(unit->unit_type) != m_army_composition_map.end())
			{
				unit_count_map[unit->unit_type]++;
				total_unit_count++;
			}
		}

		for (const auto& item : m_army_composition_map)
		{
			::sc2::UNIT_TYPEID unit_type = item.first;
			float target_proportion = item.second;

			int num_units = 0;
			for (const auto& unit : all_own_units) if (unit->unit_type == unit_type) ++num_units;

			float current_proportion = 0.0f;

			if (total_unit_count > 0)
			{
				current_proportion =
					static_cast<float>(unit_count_map[unit_type]) / static_cast<float>(total_unit_count);
			}

			/*if (current_proportion >= target_proportion)
			{
				continue;
			}*/
			// skipping proportion check because it is accounted for in _buildProductionDueToBank()

			auto trained_from = utils::_isTrainedFrom(unit_type);
			auto unit_cost = mediator.GetUnitCost(aeolusbot, unit_type);
			if (trained_from.has_value())
			{
				TechUp techUp(unit_type);
				bool techingUp = techUp.execute(aeolusbot);
				if (techingUp) return true;

				size_t existing_production_count = 0;

				for (const auto& structure : all_own_structures)
					if (structure->unit_type == trained_from.value()) existing_production_count++;
				existing_production_count += mediator.GetNumberPending(aeolusbot, trained_from.value());

				if (_buildProductionDueToBank(aeolusbot, unit_type, mineral_collection_rate, gas_collection_rate,
					existing_production_count, trained_from.value(), target_proportion))
				{
					return true;
				}
			}
		}
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
		// std::cout << "Our current mineral income is: " << mineral_collection_rate << std::endl;
		// std::cout << "Our current gas income is:" << gas_collection_rate << std::endl;

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

		bool mineral_ok = existing_production_count < rate_supported_by_minerals;
		bool gas_ok = unit_cost.second == 0 || existing_production_count < rate_supported_by_gas;

		if (mineral_ok && gas_ok)
		{
			BuildStructure build(production_structure_id, m_base_location, false);
			return build.execute(aeolusbot);
		}
		return false;
	}
}