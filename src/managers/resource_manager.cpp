#include <sc2api/sc2_interfaces.h>
#include <set>
#include <algorithm>
#include <iterator>

#include "manager_mediator.h"
#include "resource_manager.h"
#include "../utils/unit_utils.h"
#include "../utils/position_utils.h"
#include "../Aeolus.h"

namespace Aeolus
{
	std::any ResourceManager::ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args)
	{
		// std::cout << "ResourceManager: Received request" << std::endl;

		switch (request)
		{
		case constants::ManagerRequestType::GET_ALL_MINERAL_PATCHES:
		{
			return GetAllMineralPatches(aeolusbot);
		}
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
			auto params = std::any_cast<std::tuple<std::vector<::sc2::Point3D> >>(args);
			std::vector<::sc2::Point3D> expansion_locations = std::get<0>(params);
			CalculateMineralGatheringPoints(aeolusbot, expansion_locations);
			return 0;
		}
		case constants::ManagerRequestType::GET_MINERAL_GATHERING_POINTS:
			return m_mineral_gathering_points;

		case constants::ManagerRequestType::GET_WORKERS_TO_PATCH:
		{
			return m_worker_to_patch;  // This might be throwing an exception
		}
		case constants::ManagerRequestType::ASSIGN_INITIAL_WORKERS:
		{
			if (!m_initial_workers_assigned)
			{
				std::cout << "*** Initial worker assignment in process... ***" << std::endl;
				AssignInitialWorkers(aeolusbot);
				return 0;
				m_initial_workers_assigned = true;
			}
			else
			{
				std::cout << "AssignInitialWorkers: Already assigned, skipping." << std::endl;
				return 0;
			}
		}
		}
	}


	sc2::Units ResourceManager::GetAllMineralPatches(AeolusBot& aeolusbot) {
		
		// Retrieve all visible units
		const sc2::Units all_units = aeolusbot.Observation()->GetUnits(sc2::Unit::Alliance::Neutral);

		// Filter for mineral fields
		sc2::Units mineral_patches;
		for (const auto& unit : all_units) {
			if (unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD450 ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_MINERALFIELD750 ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750 ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750 ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD ||
				unit->unit_type == sc2::UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750) {
				mineral_patches.push_back(unit);
			}
		}
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

	void ResourceManager::AssignInitialWorkers(AeolusBot& aeolusbot)
	{
		::sc2::Point2D start_location_2d = utils::ConvertTo2D(aeolusbot.Observation()->GetStartLocation());
		::sc2::Units workers = ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::GATHERING);
		// std::cout << "*** Initial worker assignment in process... ***" << std::endl;
		::sc2::Units sorted_minerals = utils::SortByDistanceTo(m_all_minerals, start_location_2d);

		std::cout << "*** Initial worker assignment in process... ***" << std::endl;
		// assigned workers keeps a list of workers already assigned to mineral patch
		std::set<::sc2::Tag> assigned_workers;

		auto* debug = aeolusbot.Debug();

		// Assuming sorted_minerals is a vector of mineral patch pointers
		for (size_t i = 0; i < sorted_minerals.size(); ++i) {
			debug->DebugSphereOut(sorted_minerals[i]->pos, 1.0, sc2::Colors::Green);
			debug->DebugTextOut("Mineral " + std::to_string(i), sorted_minerals[i]->pos, sc2::Colors::Yellow);
		}

		debug->DebugSphereOut(aeolusbot.Observation()->GetStartLocation(), 3.0, sc2::Colors::Red);
		debug->SendDebug();

		// Send all the debug commands at once

		for (size_t i = 0; i < sorted_minerals.size(); ++i)
		{

			const ::sc2::Unit* mineral_patch{ sorted_minerals[i] };


			/*
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
			*/

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
		std::vector<::sc2::Point3D> expansion_locations)
	{

		// Calculate targets for Move commands towards mineral fields when speed mining.
		float mining_radius{ constants::MINING_RADIUS };

		// ::sc2::Units mineral_fields{ GetAllMineralPatches(aeolusbot) };

		::sc2::Units mineral_fields{ ManagerMediator::getInstance().GetAllMineralPatches(aeolusbot) };

		for (auto& mineral_field : mineral_fields)
		{
			::sc2::Point2D mineral_position{ utils::ConvertTo2D(mineral_field->pos) };

			/*
			fix this later:

			::sc2::Point2D mining_center{
				utils::GetClosestTo(
					utils::ConvertTo2D(mineral_field->pos),
					utils::ConvertTo2DVector(expansion_locations)
				)
			};

			*/
			::sc2::Point2D mining_center = utils::ConvertTo2D(aeolusbot.Observation()->GetStartLocation());

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

			m_mineral_gathering_points[mineral_field] = target;
		}

	}
}
