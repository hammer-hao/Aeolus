#include "build_structure.h"
#include "../../Aeolus.h"
#include "../../utils/unit_utils.h"

namespace Aeolus
{
	bool BuildStructure::execute(AeolusBot& aeolusbot)
	{
		std::cout << "BuildStructure: Executing..." << std::endl;
		bool build_within_power_field = (structure_id != ::sc2::UNIT_TYPEID::PROTOSS_PYLON);
		bool find_alternative = true;
		bool reserve_placement = true;
		std::optional<::sc2::Point2D> placement = std::nullopt;

		if (structure_id == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR)
		{
			auto* observation = aeolusbot.Observation();

			::sc2::Units existing_geysers;
			for (const auto& structure : ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot))
			{
				if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
					structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH)
				{
					existing_geysers.push_back(structure);
				}
			}
			::sc2::Units all_gas_springs = ManagerMediator::getInstance().GetAllVespeneGeysers(aeolusbot);
			::sc2::Units own_town_halls = ManagerMediator::getInstance().GetOwnTownHalls(aeolusbot);
			::sc2::Units available_gas_springs;

			for (const auto& gas : all_gas_springs)
			{
				bool valid = true;
				for (const auto& geyser : existing_geysers)
				{
					if (::sc2::DistanceSquared2D(gas->pos, geyser->pos) < 25.0f)
					{
						valid = false;
						break;
					}
				}
				if (valid == false) continue;

				for (const auto& town_hall : own_town_halls)
				{
					if (::sc2::DistanceSquared2D(town_hall->pos, gas->pos) < 144.0f
						&& town_hall->build_progress > 0.9f)
					{
						available_gas_springs.push_back(gas);
					}
				}
			}

			if (available_gas_springs.empty())
			{
				std::cout << "No geyser position available!" << std::endl;
				return false;
			}
			placement = utils::SortByDistanceTo(available_gas_springs, observation->GetStartLocation())[0]->pos;
		}
		else
		{
			placement = ManagerMediator::getInstance().RequestBuildingPlacement(
				aeolusbot,
				base_index,
				structure_id,
				is_wall,
				find_alternative,
				reserve_placement,
				build_within_power_field,
				0.8f,
				false);

			std::cout << "BuildStructure: found a valid placement" << std::endl;
		}

		if (placement.has_value())
		{
			auto worker = ManagerMediator::getInstance().SelectWorkerClosestTo(aeolusbot, placement.value());
			if (worker.has_value())
				return ManagerMediator::getInstance().BuildWithSpecificWorker(aeolusbot, worker.value(), structure_id, placement.value());
		}
		return false;
	}
}