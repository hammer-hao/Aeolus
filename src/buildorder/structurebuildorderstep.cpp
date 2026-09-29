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
				if (m_to_build == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR && !hasAvailableGasSprings(aeolusbot))
				{
					return true;
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
		auto& mediator = ManagerMediator::getInstance();

		int num_now = 0;

		for (const auto& structure : mediator.GetAllOwnStructures(aeolusbot))
		{
			if (structure->unit_type == m_to_build) num_now++;
		}
		if (num_now > m_num_before) return true;
		// wanted to build a nexus, and it is pending (with minerals reserved)
		// move on. 
		if (m_to_build == ::sc2::UNIT_TYPEID::PROTOSS_NEXUS || m_to_build == ::sc2::UNIT_TYPEID::PROTOSS_PYLON)
		{
			for (const auto& item : mediator.GetBuildingTracker(aeolusbot))
			{
				if (item.second.building_id == m_to_build)
				{
					for (const auto& structure : mediator.GetAllEnemyStructures(aeolusbot))
					{
						if (::sc2::DistanceSquared2D(item.second.target, structure->pos) < 25.0f)
						{
							// found something dangerously close to the nexus, skip the build
							return true;
						}
					}

					if (::sc2::DistanceSquared2D(item.first->pos, item.second.target) <= 9.0f 
						&& !mediator.IsGroundPositionSafe(aeolusbot, item.first->pos))
					{
						// the worker in charge is in serious trouble, let's just assume it failed to build
						// and move on.
						// of course, the construction may still take place.
						return true;
					}
				}
			}
		}
		if (m_to_build == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR && !hasAvailableGasSprings(aeolusbot))
		{
			return true;
		}
		return false;
	}

	bool StructureBuildOrderStep::started()
	{
		return m_started;
	}

	bool StructureBuildOrderStep::hasAvailableGasSprings(AeolusBot& aeolusbot)
	{
		auto* observation = aeolusbot.Observation();
		auto& mediator = ManagerMediator::getInstance();

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
					return true;
				}
			}
		}
		return false;
	}
}