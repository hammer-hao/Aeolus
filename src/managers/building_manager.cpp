#include "building_manager.h"
#include "manager.h"
#include "manager_mediator.h"
#include "../constants.h"
#include "../Aeolus.h"
#include "../utils/position_utils.h"
#include "../utils/unit_utils.h"
#include <sc2api/sc2_common.h>

namespace Aeolus
{
	std::any BuildingManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		switch (request)
		{
		case (constants::ManagerRequestType::BUILD_WITH_SPECIFIC_WORKER):
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*, ::sc2::UNIT_TYPEID, ::sc2::Point2D, bool>>(args);
			const ::sc2::Unit* worker = std::get<0>(params);
			::sc2::UNIT_TYPEID structure_id = std::get<1>(params);
			::sc2::Point2D target = std::get<2>(params);
			bool assign_role = std::get<3>(params);
			return _buildWithSpecificWorker(worker, structure_id, target, assign_role);
		}
		case (constants::ManagerRequestType::GET_NUMBER_PENDING):
		{
			auto params = std::any_cast<std::tuple<::sc2::UNIT_TYPEID>>(args);
			::sc2::UNIT_TYPEID structure_type = std::get<0>(params);
			return _getNumPending(structure_type);
		}
		default: return 0;
		}
	}

	void BuildingManager::update(int iteration)
	{
		_handleConstructionOrders();
	}

	bool BuildingManager::_buildWithSpecificWorker(const ::sc2::Unit* worker, ::sc2::UNIT_TYPEID structure_type,
		sc2::Point2D position, bool assign_role)
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();

		std::cout << "BuildingManager: Initiated new building order!" << std::endl;
		if (m_building_tracker.find(worker) == m_building_tracker.end())
		{
			// reserve the minerals for this worker
			auto cost = mediator.GetUnitCost(m_bot, structure_type);
			mediator.ReserveMinerals(m_bot, cost.first);
			mediator.ReserveVespene(m_bot, cost.second);

			m_building_tracker[worker] = { structure_type, position, 0.0, true };
			m_building_counter[structure_type] += 1;
			if (assign_role)
			{
				std::cout << "BuildingManager: Assigned BUILDING role!" << std::endl;
				ManagerMediator::getInstance().AssignRole(m_bot, worker, constants::UnitRole::BUILDING);
			}
			
			return true;
		}

		return false;
	}

	void BuildingManager::_handleConstructionOrders()
	{
		ManagerMediator& mediator = ManagerMediator::getInstance();

		::sc2::Units workers_to_remove;

		for (const auto& worker_order : m_building_tracker)
		{
			const ::sc2::Unit* worker = worker_order.first;
			BuildingOrder building_order = worker_order.second;

			// Check if we are finished with building
			auto all_structures = mediator.GetAllOwnStructures(m_bot);
			::sc2::Units close_structures = utils::GetCloserThan(all_structures, 1.5f, building_order.target);
			//if (close_structure->build_progress > 1e-16)
			if (close_structures.size() > 0)
			{
				// finished building, free the reserved minerals and vespene for this building
				workers_to_remove.push_back(worker);
				auto cost = mediator.GetUnitCost(m_bot, building_order.building_id);
				mediator.FreeMinerals(m_bot, cost.first);
				mediator.FreeVespene(m_bot, cost.second);
				continue;
			}
			
			if (constants::GAS_BUILDINGS.find(building_order.building_id) != constants::GAS_BUILDINGS.end())
			{
				float distance_threshold = 20.25f;

				// if the worker is too far, move to target first
				if (::sc2::DistanceSquared2D(worker->pos, building_order.target) > distance_threshold)
				{
					if (worker->orders.size() > 0)
					{
						if (worker->orders[0].target_pos == building_order.target) continue;
					}
					m_bot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::MOVE_MOVE, building_order.target);
				}
				// When the worker arrive, issue the build command
				else if (worker->orders.size() == 0)
				{
					::sc2::Units all_vespene_geysers = ManagerMediator::getInstance().GetAllVespeneGeysers(m_bot);
					if (all_vespene_geysers.empty()) return;

					auto target_geyser = utils::SortByDistanceTo(all_vespene_geysers, building_order.target)[0];
					::sc2::ABILITY_ID build_command = ManagerMediator::getInstance().GetCreationAbility(m_bot, building_order.building_id);
					m_bot.Actions()->UnitCommand(worker, build_command, target_geyser);
				}
			}
			else
			{
				float distance_threshold = 1.0f;

				// if the worker is too far, move to target first
				if (::sc2::DistanceSquared2D(worker->pos, building_order.target) > distance_threshold)
				{
					if (worker->orders.size() > 0)
					{
						if (worker->orders[0].target_pos == building_order.target) continue;
					}
					m_bot.Actions()->UnitCommand(worker, ::sc2::ABILITY_ID::MOVE_MOVE, building_order.target);
				}

				// When the worker arrive, issue the build command
				else if (worker->orders.size() == 0)
				{
					::sc2::ABILITY_ID build_command = ManagerMediator::getInstance().GetCreationAbility(m_bot, building_order.building_id);
					m_bot.Actions()->UnitCommand(worker, build_command, building_order.target);
				}
			}
		}

		for (const auto& worker : workers_to_remove)
		{
			m_building_counter[m_building_tracker[worker].building_id] -= 1;
			m_building_tracker.erase(worker);
			ManagerMediator::getInstance().AssignRole(m_bot, worker, constants::UnitRole::GATHERING);
		}
	}

	size_t BuildingManager::_getNumPending(::sc2::UNIT_TYPEID structure_type)
	{
		size_t i = 0;
		for (const auto& worker_order : m_building_tracker)
		{
			if (worker_order.second.building_id == structure_type) i++;
		}
		return i;
	}

	void BuildingManager::OnUnitDestroyed(const ::sc2::Unit* unit)
	{
		// check if the destroyed unit is a worker with assigned building
		auto it = m_building_tracker.find(unit);
		if (it != m_building_tracker.end())
		{
			// a worker with assigned building is destroyed
			// clear it from the building tracker, and send out a new worker to replace it
			ManagerMediator& mediator = ManagerMediator::getInstance();

			auto worker = mediator.SelectWorkerClosestTo(m_bot, it->second.target);

			if (!worker.has_value())
			{
				auto cost = mediator.GetUnitCost(m_bot, it->second.building_id);
				mediator.FreeMinerals(m_bot, cost.first);
				mediator.FreeVespene(m_bot, cost.second);
				m_building_counter[m_building_tracker[unit].building_id] -= 1;
				m_building_tracker.erase(unit);
			}
			else
			{
				BuildingOrder order = it->second; // extract the existing building order
				m_building_tracker.erase(unit); // erase the previous unit
				m_building_tracker[worker.value()] = order;

				std::cout << "BuildingManager: Assigned BUILDING role!" << std::endl;
				ManagerMediator::getInstance().AssignRole(m_bot, worker.value(), constants::UnitRole::BUILDING);
			}
		}
	}
}