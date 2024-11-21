#include "neutral_unit_manager.h"
#include "../Aeolus.h"
#include "../constants.h"

#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	std::any NeutralUnitManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		std::cout << "NeutralUnitManager: recieved request" << std::endl;
		switch (request)
		{
		case constants::ManagerRequestType::GET_ALL_MINERAL_PATCHES:
		{
			return GetAllMineralPatches();
		}
		case constants::ManagerRequestType::GET_ALL_VESPENE_GEYSERS:
		{
			return GetAllVespeneGeysers();
		}
		case constants::ManagerRequestType::GET_ALL_DESTRUCTABLES:
		{
			return GetAllDestructables();
		}
		}
	}


	std::map<std::string_view, ::sc2::Units> NeutralUnitManager::_GetNeutralUnits(AeolusBot& aeolusbot)
	{

		::sc2::Units all_units = aeolusbot.Observation()->GetUnits(sc2::Unit::Alliance::Neutral);

		std::map<std::string_view, ::sc2::Units> neutral_units;

		for (const auto& unit : all_units)
		{
			if (constants::MINERAL_IDS.find(unit->unit_type) != constants::MINERAL_IDS.end())
			{
				neutral_units["Minerals"].push_back(unit);
			}
			else if (constants::VESPENE_IDS.find(unit->unit_type) != constants::VESPENE_IDS.end())
			{
				neutral_units["Vespenes"].push_back(unit);
			}
			else if (constants::WATCHTOWER_IDS.find(unit->unit_type) == constants::WATCHTOWER_IDS.end())
			{
				neutral_units["Destructables"].push_back(unit);
			}
		}

		return neutral_units;
	}
}