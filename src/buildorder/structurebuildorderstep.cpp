#include "structurebuildorderstep.h"
#include "../Aeolus.h"
#include "../behaviors/macro_behaviors/build_structure.h"
#include "../managers/manager_mediator.h"
#include <string>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	StructureBuildOrderStep::StructureBuildOrderStep(int supply_threshold, ::sc2::UNIT_TYPEID to_build, bool is_wall,
		int base_location) :
		m_supply_threshold(supply_threshold), m_to_build(to_build), m_is_wall(is_wall), m_started(false), m_num_before(0),
		m_base_location(base_location)
	{
	}

	int StructureBuildOrderStep::getSupplyThreshold()
	{
		return m_supply_threshold;
	}

	bool StructureBuildOrderStep::execute(AeolusBot& aeolusbot)
	{
		if (aeolusbot.Observation()->GetFoodUsed() >= m_supply_threshold)
		{
			m_num_before = 0;
			for (const auto& structure : ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot))
			{
				if (structure->unit_type == m_to_build) m_num_before++;
			}
			std::make_unique<BuildStructure>(m_to_build, m_base_location, m_is_wall).get()->execute(aeolusbot);
			m_started = true;
			return true;
		}
		return false;
	}

	std::string_view StructureBuildOrderStep::toString()
	{
		return ::sc2::UnitTypeToName(m_to_build);
	}

	bool StructureBuildOrderStep::isDone(AeolusBot& aeolusbot)
	{
		int num_now = 0;
		for (const auto& structure : ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot))
		{
			if (structure->unit_type == m_to_build) num_now++;
		}
		return (num_now > m_num_before);
	}

	bool StructureBuildOrderStep::started()
	{
		return m_started;
	}


}