#include "unit_filter_manager.h"
#include "../Aeolus.h"
#include <sc2api/sc2_interfaces.h>

namespace Aeolus
{
	std::any UnitFilterManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::GET_ALL_STRUCTURES):
		{
			auto params = std::any_cast<std::tuple<::sc2::Unit::Alliance>>(args);
			const ::sc2::Unit::Alliance alliance = std::get<0>(params);
			return _getAllStructures(aeolusbot, alliance);
		}
		default:
			return 0;
		}
	}

	::sc2::Units UnitFilterManager::_getAllStructures(AeolusBot& aeolusbot, ::sc2::Unit::Alliance alliance)
	{
		const ::sc2::ObservationInterface* observation = aeolusbot.Observation();
		return observation->GetUnits(
			alliance, 
			[observation](const ::sc2::Unit& unit)
			{
				const ::sc2::UnitTypeData& unit_type_data = observation->GetUnitTypeData()[unit.unit_type];
				return constants::BUILDINGS.find(unit.unit_type) != constants::BUILDINGS.end();
				/*
				return (unit_type_data.movement_speed == 0.0f && // Structures don't move
				unit_type_data.build_time > 0.0f && // Structures take time to build
				unit_type_data.food_required == 0.0f); // Structures don't consume supply
				*/
			});
	}
}