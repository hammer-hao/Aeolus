#include "auto_supply.h"
#include "macro_behavior.h"
#include "build_structure.h"
#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	bool AutoSupply::execute(AeolusBot& aeolusbot)
	{
		if (_numSupplyRequired(aeolusbot) > 0)
		{
			std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_PYLON, m_base_location, false).get()->execute(aeolusbot);
			return true;
		}
		return false;
	}

	int AutoSupply::_numSupplyRequired(AeolusBot& aeolusbot)
	{
		auto* observation = aeolusbot.Observation();
		auto& mediator = ManagerMediator::getInstance();
		int current_cap = observation->GetFoodCap();
		if (current_cap >= 200) return 0;

		int num_nexi = mediator.GetOwnTownHalls(aeolusbot).size();
		int pending_pylons = mediator.GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_PYLON);
		for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON && structure->build_progress < 1.0f) pending_pylons++;
		int supply_left = current_cap - observation->GetFoodUsed();

		// get all production structures
		int num_production_structures = 0;
		for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
		{
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_GATEWAY
				|| structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_WARPGATE
				|| structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY
				|| structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_STARGATE)
				num_production_structures++;
		}

		if (supply_left <= std::max(2 * num_production_structures, 5))
		{
			// std::cout << "Number of pending pylons: " << pending_pylons << " production structures: "
			//	<< std::ceil(static_cast<float>(num_production_structures) / 2) << std::endl;
			if (pending_pylons < std::ceil(static_cast<float>(num_production_structures) / 2))
			{
				int num = static_cast<int>(std::ceil(static_cast<float>(num_production_structures) / 2) - pending_pylons);
				return std::min(num, 6);
			}
		}
		else if (num_production_structures == 0 && supply_left <= 2 && pending_pylons == 0)
		{
			return 1;
		}

		return 0;
	}
}