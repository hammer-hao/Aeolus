#include "expand.h"
#include "../../Aeolus.h"
#include "../../managers/manager_mediator.h"

namespace Aeolus
{
	bool Expand::execute(AeolusBot& aeolusbot)
	{
		if (aeolusbot.Observation()->GetMinerals() < 400) return false;

		size_t num_pending = ManagerMediator::getInstance().GetNumberPending(aeolusbot, ::sc2::UNIT_TYPEID::PROTOSS_NEXUS);

		if (ManagerMediator::getInstance().GetOwnTownHalls(aeolusbot).size() + num_pending >= m_to_count
			|| num_pending >= m_max_pending) 
			return false;

		std::cout << "Expand: solving next expansion location... " << std::endl;
		
		auto target_location = _getNextExpansionLocation(aeolusbot);

		if (target_location == ::sc2::Point2D(0, 0)) return false;

		std::cout << "Expand: Selecting a worker... " << std::endl;

		auto worker = ManagerMediator::getInstance().SelectWorkerClosestTo(aeolusbot, target_location);

		if (worker.has_value())
		{
			std::cout << "Expand: Building with the worker.. " << std::endl;
			return ManagerMediator::getInstance().BuildWithSpecificWorker(aeolusbot, 
				worker.value(), ::sc2::UNIT_TYPEID::PROTOSS_NEXUS, target_location);
		}
		return false;
	}

	::sc2::Point2D Expand::_getNextExpansionLocation(AeolusBot& aeolusbot)
	{
		auto els = ManagerMediator::getInstance().GetExpansionLocations(aeolusbot);
		for (const auto& el : els)
		{
			if (!ManagerMediator::getInstance().IsGroundPositionSafe(aeolusbot, el)) continue;

			if (_locationIsBlocked(aeolusbot, el)) continue;

			return el;
		}
		return { 0, 0 };
	}

	bool Expand::_locationIsBlocked(AeolusBot& aeolusbot, ::sc2::Point2D location)
	{
		std::vector<::sc2::Point2D> locations_to_check;
		locations_to_check.push_back(location);
		std::cout << "Expand: checking if position is blocked by enemies... " << std::endl;
		auto close_enemy = ManagerMediator::getInstance().GetUnitsInRange(aeolusbot, locations_to_check, 5.5f);
		std::cout << "Expand: check complete. " << std::endl;
		if (!close_enemy.empty()) return true;

		// check if we already have a nexus here
		std::cout << "Expand: checking if position is blocked by our nexus... " << std::endl;
		auto close_own = ManagerMediator::getInstance().GetOwnUnitsInRange(aeolusbot, locations_to_check, 5.5f);
		for (const auto& unit : close_own)
		{
			if (unit->unit_type == static_cast<int>(::sc2::UNIT_TYPEID::PROTOSS_NEXUS)) return true;
		}
		std::cout << "Expand: check complete. " << std::endl;
		return false;
	}
}