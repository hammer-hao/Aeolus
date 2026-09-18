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
		// fix for https://github.com/hammer-hao/Aeolus/issues/29 -- just execute every other step to
		// avoid game client not properly updating the action result
		if (aeolusbot.Observation()->GetGameLoop() % 2 == 0) return false;

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

		std::vector<std::pair<::sc2::UNIT_TYPEID, float>> deficits;

		for (const auto& item : m_army_composition_map)
		{
			::sc2::UNIT_TYPEID unit_type = item.first;
			float target_proportion = item.second;

			float current_proportion = 0.0f;

			if (total_unit_count > 0)
			{
				current_proportion =
					static_cast<float>(unit_count_map[unit_type]) /
					static_cast<float>(total_unit_count);
			}

			float deficit = target_proportion - current_proportion;

			deficits.push_back({ unit_type, deficit });
		}
		std::sort(
			deficits.begin(),
			deficits.end(),
			[](const auto& a, const auto& b)
			{
				return a.second > b.second;
			});

		bool tech_up_attempted = false;

		for (const auto& candidate : deficits)
		{
			::sc2::UNIT_TYPEID unit_type = candidate.first;

			float target_proportion = m_army_composition_map.at(unit_type);

			auto trained_from = utils::_isTrainedFrom(unit_type);

			if (!trained_from.has_value())
				continue;

			::sc2::UNIT_TYPEID required_tech = mediator.GetRequiredTech(aeolusbot, unit_type);
			bool tech_ready = false; // start false
			for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
			{
				if (structure->unit_type == required_tech && structure->build_progress >= 1.0f)
				{
					auto prod = utils::_isTrainedFrom(unit_type).value();

					// if warpgate is ready, change prod to warpgate
					auto upgrades = observation->GetUpgrades();
					if (prod == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY &&
						std::find(upgrades.begin(), upgrades.end(), ::sc2::UPGRADE_ID::WARPGATERESEARCH)
						!= upgrades.end())
						prod = ::sc2::UNIT_TYPEID::PROTOSS_WARPGATE;
					tech_ready = true;
					break;
				}
			}

			if (!tech_ready)
			{
				// Only allow one TechUp attempt during this execution.
				if (!tech_up_attempted)
				{
					TechUp techUp(unit_type);
					techUp.execute(aeolusbot);
					tech_up_attempted = true;
				}
				// Whether this was the unit we tried to tech for or some
				// later unit whose tech is also unavailable, skip it.
				continue;
			}

			size_t existing_production_count = 0;

			for (const auto& structure : all_own_structures)
			{
				if (structure->unit_type == trained_from.value())
				{
					existing_production_count++;
				}
				else if (
					trained_from.value() == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY &&
					structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPGATE)
				{
					existing_production_count++;
				}
			}

			existing_production_count +=
				mediator.GetNumberPending(aeolusbot, trained_from.value());

			auto result = _buildProductionDueToBank(
				aeolusbot,
				unit_type,
				mineral_collection_rate,
				gas_collection_rate,
				existing_production_count,
				trained_from.value(),
				target_proportion);
			switch (result)
			{
			case ProductionBuildResult::Built:
				return true;
			case ProductionBuildResult::Blocked:
				// This is the highest-priority candidate that actually
				// needs production. Don't fall through and spend resources
				// on a lower-priority/cheaper structure.
				return false;
			case ProductionBuildResult::NotNeeded:
				// This candidate is satisfied. Check the next deficit.
				continue;
			}
			
		}
		return false;
	}

	ProductionBuildResult ProductionController::_buildProductionDueToBank(AeolusBot& aeolusbot,
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

		// We don't need more production for this unit.
		if (!mineral_ok || !gas_ok)
			return ProductionBuildResult::NotNeeded;

		// We DO need more production.
		BuildStructure build(
			production_structure_id,
			m_base_location,
			false
		);

		if (build.execute(aeolusbot))
			return ProductionBuildResult::Built;

		// We need it, but can't build it right now.
		return ProductionBuildResult::Blocked;
	}
}