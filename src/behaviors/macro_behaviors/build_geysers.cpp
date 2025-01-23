#include "build_geysers.h"
#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"
#include "../../utils/unit_utils.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>
#include <vector>

namespace Aeolus
{
	bool BuildGeysers::execute(AeolusBot& aeolusbot)
	{
		auto* observation = aeolusbot.Observation();
		if (m_smart_gas && (static_cast<int>(observation->GetMinerals()) - static_cast<int>(observation->GetVespene())) < m_smart_gas_threshold)
			return false;
		
		::sc2::Units existing_geysers;
		int pending_geysers = 0;
		int active_geysers = 0;
		pending_geysers = pending_geysers +
			ManagerMediator::getInstance().GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR);
		for (const auto& structure : ManagerMediator::getInstance().GetAllOwnStructures(aeolusbot))
		{
			if (structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
				structure->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATORRICH)
			{
				existing_geysers.push_back(structure);
				if (structure->vespene_contents > 0) active_geysers++;
				if (structure->build_progress < 1.0f) pending_geysers++;
			}
		}

		if (active_geysers > m_to_active_count || pending_geysers > m_max_pending) return false;

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
		::sc2::Point2D build_target = utils::SortByDistanceTo(available_gas_springs, observation->GetStartLocation())[0]->pos;

		auto build_worker = ManagerMediator::getInstance().SelectWorkerClosestTo(aeolusbot, build_target);
		if (build_worker.has_value())
			ManagerMediator::getInstance().BuildWithSpecificWorker(aeolusbot, 
				build_worker.value(), ::sc2::UNIT_TYPEID::PROTOSS_ASSIMILATOR, build_target);
		// yay!
		return true;
	}
}