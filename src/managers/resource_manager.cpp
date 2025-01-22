#include <sc2api/sc2_interfaces.h>
#include <set>
#include <algorithm>
#include <iterator>
#include <optional>
#include <chrono> 
#include <sc2lib/sc2_search.h>
#include <sc2api/sc2_map_info.h>
#include <sc2api/sc2_common.h>

#include "manager_mediator.h"
#include "resource_manager.h"
#include "../utils/unit_utils.h"
#include "../utils/position_utils.h"
#include "../utils/game_utils.h"
#include "../Aeolus.h"

namespace Aeolus
{
	void ResourceManager::update(int iteration)
	{
		if (!m_initial_workers_assigned)
		{
			std::cout << "assigning initial workers... " << std::endl;
			// CalculateMineralGatheringPoints(m_bot, ::sc2::search::CalculateExpansionLocations(m_bot.Observation(), m_bot.Query()));
			CalculateMineralGatheringPoints(m_bot, ManagerMediator::getInstance().GetExpansionLocations(m_bot));
			AssignInitialWorkers();
			m_initial_workers_assigned = true;
		}
		else
		{
			::sc2::Units all_workers = ManagerMediator::getInstance().GetUnitsFromRole(m_bot, constants::UnitRole::GATHERING);
			::sc2::Units unassigned_workers;
			::sc2::Units own_town_halls = ManagerMediator::getInstance().GetOwnTownHalls(m_bot);
			if (all_workers.empty() || own_town_halls.empty()) return;
			else
			{
				::sc2::Units available_minerals = GetAvailableMinerals();

				::sc2::Units all_gas_buildings = ManagerMediator::getInstance().GetOwnGasBuildings(m_bot);

				for (const auto& worker : all_workers)
				{
					if (m_worker_to_patch.find(worker) == m_worker_to_patch.end()) unassigned_workers.push_back(worker);
				}

				if (!all_gas_buildings.empty())
				{
					auto start_time = std::chrono::high_resolution_clock::now();
					_assignWorkersToGasBuildings(unassigned_workers, all_gas_buildings);
					auto end_time = std::chrono::high_resolution_clock::now();
					auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
					if (iteration % 100 == 0) std::cout <<
						"Number of gas buildings: " << all_gas_buildings.size() << "Elapsed_ms: " << elapsed_ms << std::endl;
				}

				// std::cout << "Available patches:" << available_minerals.size() << std::endl;
				if (!available_minerals.empty())
				{

					// AssignWorkersToMineralPatches(unassigned_workers, available_minerals);
					// std::cout << "Unassigned workers: " << unassigned_workers.size() << std::endl;
					_assignWorkersToMineralPatches(unassigned_workers, available_minerals);
					// std::cout << "Assigned unassigned workers to available patches!" << std::endl;
				}
			}
		}
	}

	std::any ResourceManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		// std::cout << "ResourceManager: Received request" << std::endl;

		switch (request)
		{
		case constants::ManagerRequestType::ASSIGN_WORKER_TO_PATCH:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*, const::sc2::Unit*>>(args);
			const ::sc2::Unit* worker = std::get<0>(params);
			const ::sc2::Unit* patch = std::get<1>(params);
			AssignWorkerToPatch(worker, patch);
			return 0;
		}
		case constants::ManagerRequestType::CLEAR_ASSIGNMENT:
		{
			auto params = std::any_cast<std::tuple<const ::sc2::Unit*>>(args);
			const ::sc2::Unit* unit = std::get<0>(params);
			ClearAssignment(unit);
			return 0;
		}
		case constants::ManagerRequestType::CALCULATE_MINERAL_GATHERING_POINTS:
		{
			auto params = std::any_cast<std::tuple<std::vector<::sc2::Point2D> >>(args);
			std::vector<::sc2::Point2D> expansion_locations = std::get<0>(params);
			CalculateMineralGatheringPoints(aeolusbot, expansion_locations);
			return 0;
		}
		case constants::ManagerRequestType::GET_MINERAL_GATHERING_POINTS:
			return m_mineral_gathering_points;

		case constants::ManagerRequestType::GET_WORKERS_TO_PATCH:
		{
			return m_worker_to_patch;  // This might be throwing an exception
		}
		case constants::ManagerRequestType::GET_WORKERS_TO_GEYSER:
		{
			return m_worker_to_geyser;
		}
		case constants::ManagerRequestType::ASSIGN_INITIAL_WORKERS:
		{
			if (!m_initial_workers_assigned)
			{
				std::cout << "*** Initial worker assignment in process... ***" << std::endl;
				AssignInitialWorkers();
				return 0;
				m_initial_workers_assigned = true;
			}
			else
			{
				std::cout << "AssignInitialWorkers: Already assigned, skipping." << std::endl;
				return 0;
			}
		}
		case (constants::ManagerRequestType::SELECT_WORKER_TO_TARGET):
		{
			auto params = std::any_cast<std::tuple<::sc2::Point2D>>(args);
			::sc2::Point2D target_location = std::get<0>(params);
			return _selectWorker(target_location);
		}
		default: return 0;
		}
	}


	sc2::Units ResourceManager::_getAllMineralPatches(AeolusBot& aeolusbot) {

		::sc2::Units mineral_patches{ ManagerMediator::getInstance().GetAllMineralPatches(aeolusbot) };
		m_all_minerals = mineral_patches;

		return mineral_patches;
	}

	void ResourceManager::AssignWorkerToPatch(const ::sc2::Unit* worker, const ::sc2::Unit* patch)
	{
		ClearAssignment(worker);
		m_patch_to_workers[patch].push_back(worker);
		m_worker_to_patch[worker] = patch;
	}

	void ResourceManager::ClearAssignment(const ::sc2::Unit* worker)
	{
		auto it = m_worker_to_patch.find(worker);
		if (it != m_worker_to_patch.end())
		{
			const ::sc2::Unit* patch = it->second;
			auto& workers = m_patch_to_workers[patch];
			workers.erase(std::remove(workers.begin(), workers.end(), worker), workers.end());

			m_worker_to_patch.erase(it);
		}
	}

	void ResourceManager::AssignInitialWorkers()
	{
		::sc2::Point2D start_location_2d = utils::ConvertTo2D(m_bot.Observation()->GetStartLocation());
		::sc2::Units workers = ManagerMediator::getInstance().GetOwnWorkers(m_bot);
		// std::cout << "*** Initial worker assignment in process... ***" << std::endl;
		::sc2::Units sorted_minerals = utils::SortByDistanceTo(m_all_minerals, start_location_2d);

		std::cout << "*** Initial worker assignment in process... ***" << std::endl;
		// assigned workers keeps a list of workers already assigned to mineral patch
		std::set<::sc2::Tag> assigned_workers;

		// Send all the debug commands at once

		for (size_t i = 0; i < sorted_minerals.size(); ++i)
		{

			const ::sc2::Unit* mineral_patch{ sorted_minerals[i] };

			// Determine how many workers to assign to this mineral patch
			size_t take = (i < 4) ? 2 : 1;  // First 3 mineral patches get 2 workers, rest get 1
			// now take = 2 if this mineral patch is close, take = 1 if not


			// now we assign the workers
			for (size_t j = 0; j < take; ++j) // limit by sorted_unassigned_workers.size() when there are not enough workers left.
			{
				// Filter workers that have not been assigned yet, put them into leftover_workers
				::sc2::Units leftover_workers;
				std::copy_if
				(
					workers.begin(), workers.end(),
					std::back_inserter(leftover_workers),
					[&assigned_workers](const ::sc2::Unit* worker)
					{
						// if the worker's tag is not in assigned list, we know it is free to be assigned
						return assigned_workers.find(worker->tag) == assigned_workers.end();
					}
				);

				::sc2::Units sorted_unassigned_workers = utils::SortByDistanceTo
				(
					leftover_workers,
					utils::ConvertTo2D(mineral_patch->pos)
				);

				if (sorted_unassigned_workers.size() < 1)
				{
					return; //finisehd assignment
				}

				const ::sc2::Unit* worker = sorted_unassigned_workers[j]; // find the closest candidates to the mineral patch
				AssignWorkerToPatch(worker, mineral_patch);
				assigned_workers.insert(worker->tag);
				std::cout << "Assigned an Initial worker!" << std::endl;
			}
		}

		std::stringstream debugMessage;
		debugMessage << " <<< Assigned: " << assigned_workers.size() << " Workers >>>";
		std::cout << debugMessage.str() << std::endl;
	}

	void ResourceManager::CalculateMineralGatheringPoints(
		AeolusBot& aeolusbot,
		std::vector<::sc2::Point2D> expansion_locations)
	{

		// Calculate targets for Move commands towards mineral fields when speed mining.
		float mining_radius{ constants::MINING_RADIUS };

		::sc2::Units mineral_fields{ _getAllMineralPatches(aeolusbot) };

		for (auto& mineral_field : mineral_fields)
		{
			::sc2::Point2D mineral_position{ mineral_field->pos };

			::sc2::Point2D mining_center{
				utils::GetClosestTo(
					mineral_field->pos,
					expansion_locations
				)
			};

			// ::sc2::Point2D mining_center = utils::ConvertTo2D(aeolusbot.Observation()->GetStartLocation());

			::sc2::Point2D target{ utils::GetPositionTowards(
				mineral_position, 
				mining_center, 
				mining_radius) 
			};

			::sc2::Units closer_patches{ utils::GetCloserThan(
				mineral_fields,
				mining_radius,
				target
				)};

			for (auto& patch : closer_patches)
			{
				if (patch->tag != mineral_field->tag)
				{
					std::vector<::sc2::Point2D> intersection_points =
						utils::GetCircleIntersection
						(
							utils::ConvertTo2D(patch->pos),
							utils::ConvertTo2D(mineral_field->pos),
							mining_radius,
							mining_radius
						);
					
					if (intersection_points.size() == 2)
					{
						target = utils::GetClosestTo(mining_center, intersection_points);
					}
					
				}
			}

			m_mineral_gathering_points[{mineral_field->pos.x, mineral_field->pos.y}] = target;
		}
	}

	::sc2::Units ResourceManager::GetAvailableMinerals()
	{
		::sc2::Units available_minerals;
		::sc2::Units own_town_halls = ManagerMediator::getInstance().GetOwnTownHalls(m_bot);
		if (own_town_halls.empty()) return available_minerals;
		else 
		{
			::sc2::Units all_mineral_fields = ManagerMediator::getInstance().GetAllMineralPatches(m_bot);
			for (const auto& town_hall : own_town_halls)
			{
				::sc2::Units townhall_minerals;
				for (const auto& mineral_field : all_mineral_fields)
				{
					if (mineral_field->display_type == ::sc2::Unit::DisplayType::Visible
						&& ::sc2::Distance2D(::sc2::Point2D(mineral_field->pos), ::sc2::Point2D(town_hall->pos)) <= 10
						&& m_patch_to_workers[mineral_field].size() < 2)
					{
						townhall_minerals.push_back(mineral_field);
					}
				}
				if (!townhall_minerals.empty())
				{
					::sc2::Units sorted_townhall_minerals = utils::SortByDistanceTo(townhall_minerals, ::sc2::Point2D(town_hall->pos));
					available_minerals.insert(available_minerals.end(), sorted_townhall_minerals.begin(), sorted_townhall_minerals.end());
				}
			}
		}
		return available_minerals;
	}

	void ResourceManager::OnUnitDestroyed(const ::sc2::Unit* unit)
	{
		auto it = m_worker_to_patch.find(unit);
		if (it != m_worker_to_patch.end())
		{
			ClearAssignment(unit);
		}
	}

	void ResourceManager::_assignWorkersToGasBuildings(const ::sc2::Units& workers, const ::sc2::Units& gas_buildings)
	{
		constexpr int c_num_per_gas = 3;
		auto own_town_halls = ManagerMediator::getInstance().GetOwnTownHalls(m_bot);
		if (own_town_halls.empty()) return;

		// for each gas building, assign worker one at a time.
		for (const auto& gas_building : gas_buildings)
		{
			if (m_geyser_to_workers[gas_building].size() >= c_num_per_gas) continue;
			if (gas_building->build_progress >= 1.0f
				&& !utils::GetCloserThan(own_town_halls, 12, gas_building->pos).empty())
			{
				std::cout << "ResourceManager: Selecting a worker" << std::endl;
				auto worker = _selectWorker(gas_building->pos);
				if (!worker.has_value()) continue;
				std::cout << "ResourceManager: Selected a worker!" << std::endl;
				if (std::find(m_geyser_to_workers[gas_building].begin(), 
					m_geyser_to_workers[gas_building].end(), 
					worker.value()) 
					!=
					m_geyser_to_workers[gas_building].end()) continue;
				std::cout << "ResourceManager: Adding worker to gas!" << std::endl;
				m_geyser_to_workers[gas_building].push_back(worker.value());
				m_worker_to_geyser[worker.value()] = gas_building;
				_removeWorkerFromMineral(worker.value());
			}
		}
	}

	void ResourceManager::_assignWorkersToMineralPatches(::sc2::Units workers, ::sc2::Units patches)
	{
		for (const auto& worker : workers)
		{
			if (patches.empty()) return;
			if (m_worker_to_patch.find(worker) != m_worker_to_patch.end()) continue;

			// TODO: improve this for mid and late game!
			const ::sc2::Unit* mineral = utils::GetClosestUnitTo(::sc2::Point2D(worker->pos), patches);

			if (m_patch_to_workers[mineral].size() < 2)
			{
				AssignWorkerToPatch(worker, mineral);
			}

			if (m_patch_to_workers[mineral].size() >= 2)
			{
				patches.erase(std::remove(patches.begin(), patches.end(), mineral));
			}
		}
	}

	void ResourceManager::_removeWorkerFromMineral(const ::sc2::Unit* worker)
	{
		auto it = m_worker_to_patch.find(worker);
		if (it != m_worker_to_patch.end())
		{
			auto patch = it->second;
			m_worker_to_patch.erase(worker);
			m_patch_to_workers[patch].erase(std::remove(m_patch_to_workers[patch].begin(),
				m_patch_to_workers[patch].end(), worker));
		}
	}

	std::optional<const ::sc2::Unit*> ResourceManager::_selectWorker(::sc2::Point2D target_position)
	{
		::sc2::Units all_workers = ManagerMediator::getInstance().GetUnitsFromRole(m_bot, constants::UnitRole::GATHERING);
		::sc2::Units all_available_workers;

		for (const auto& worker : all_workers)
		{
			if (!utils::IsWorkerCarryingResource(worker)
				&& m_worker_to_patch.find(worker) != m_worker_to_patch.end()
				&& m_worker_to_geyser.find(worker) == m_worker_to_geyser.end())
				all_available_workers.push_back(worker);
		}
		
		if (all_available_workers.empty()) return std::nullopt;

		auto closest_worker = utils::GetClosestUnitTo(target_position, all_available_workers);
		if (closest_worker == nullptr) return std::nullopt;
		return std::optional<const sc2::Unit*>{closest_worker};
	}
}
