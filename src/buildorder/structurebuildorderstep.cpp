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

			::sc2::Units allStructures = ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot);
			for (const auto& structure : allStructures)
			{
				if (structure->unit_type == m_to_build) m_num_before++;
			}
			if (!std::make_unique<BuildStructure>(m_to_build, m_base_location, m_is_wall).get()->execute(aeolusbot))
			{
				// if what we are trying to build was not a nexus / assimilator / pylon, then it means
				// no available position at the base we are in
				// in that case, let's build a pylon first
				if (m_to_build != ::sc2::UNIT_TYPEID::PROTOSS_NEXUS &&
					m_to_build != ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR &&
					m_to_build != ::sc2::UNIT_TYPEID::PROTOSS_PYLON)
				{
					int numPendingPylons = ManagerMediator::getInstance().GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_PYLON);
					if (numPendingPylons == 0)
					{
						for (const auto& structure : allStructures)
						{
							if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_PYLON && structure->build_progress < 1.0f)
							{
								numPendingPylons++;
								break;
							}
						}
						if (numPendingPylons == 0)
						{
							// no pylons queued / in progress, build one
							std::make_unique<BuildStructure>(::sc2::UNIT_TYPEID::PROTOSS_PYLON, m_base_location, m_is_wall).get()->execute(aeolusbot);
						}
					}
				}
				return false;
			}
			else
			{
				m_started = true;
				return true;
			}
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