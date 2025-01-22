#include "build_structure.h"
#include "../../Aeolus.h"

namespace Aeolus
{
	bool BuildStructure::execute(AeolusBot& aeolusbot)
	{
		std::cout << "BuildStructure: Executing..." << std::endl;
		bool build_within_power_field = (structure_id != ::sc2::UNIT_TYPEID::PROTOSS_PYLON);
		bool find_alternative = true;
		bool reserve_placement = true;

		auto placement = ManagerMediator::getInstance().RequestBuildingPlacement(
			aeolusbot,
			base_index,
			structure_id,
			is_wall,
			find_alternative,
			reserve_placement,
			build_within_power_field);

		std::cout << "BuildStructure: found a valid placement" << std::endl;

		if (placement.has_value())
		{
			auto worker = ManagerMediator::getInstance().SelectWorkerClosestTo(aeolusbot, placement.value());
			if (worker.has_value())
				return ManagerMediator::getInstance().BuildWithSpecificWorker(aeolusbot, worker.value(), structure_id, placement.value());
		}
		return false;
	}
}