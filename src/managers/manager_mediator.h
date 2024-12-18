#pragma once

#include <iostream>
#include <any>
#include <map>

#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_map_info.h>

#include "../constants.h"
#include "manager.h"

namespace Aeolus
{
	class Manager;
	class AeolusBot;
	class ManagerMediator
	{
		/* The single source of information and coordinator
		of communication between the managers */
	private:
		ManagerMediator() = default;

		std::map<std::string, std::unique_ptr<Manager>> m_managers;

	public:

		ManagerMediator(const ManagerMediator&) = delete;
		ManagerMediator& operator=(const ManagerMediator&) = delete;

		// Public static method to get the singleton instance.
		static ManagerMediator& getInstance() {
			static ManagerMediator instance; // Guaranteed to be thread-safe.
			return instance;
		}

		void AddManagers(std::vector<std::unique_ptr<Manager>>& managers);

		// Send a request to a specific manager
		template <typename ReturnType, typename... Args>
		ReturnType ManagerRequest(
			AeolusBot& aeolusbot,
			const constants::ManagerName receiver,
			const constants::ManagerRequestType request,
			const Args&... args)
		{
			std::string manager_name{ constants::ManagerNameToString(receiver) };

			/*
			std::cout << "Checking available managers..." << std::endl;
			for (const auto& pair : m_managers)
			{
				std::cout << pair.first << std::endl;
			}			
			*/

			auto it{ m_managers.find(manager_name) };
			if (it == m_managers.end()) {
				throw std::runtime_error("Manager not found: " + manager_name);
			}

			// std::cout << "Sending request... " << std::endl;

			// Call the manager's ProcessRequest function, expanding the parameter pack
			std::any packed_args = std::make_tuple(args...);

			std::any result = it->second->ProcessRequest(aeolusbot, request, packed_args);

			try {
				return std::any_cast<ReturnType>(result);
			}
			catch (const std::bad_any_cast& e) {
				throw std::runtime_error("Failed to cast result in ManagerRequest: " + std::string(e.what()));
			}
		}


		// Manager mediator interface

		::sc2::Units GetUnitsFromRole(AeolusBot& aeolusbot, constants::UnitRole role)
		{
			// std::cout << "ManagerMediator: sending GetUnitsFromRole request to UnitRoleManager..." << std::endl;
			return ManagerRequest<::sc2::Units, constants::UnitRole>
				(
					aeolusbot,
					constants::ManagerName::UNIT_ROLE_MANAGER,
					constants::ManagerRequestType::GET_UNITS_FROM_ROLE,
					role
				);
		};

		int CatchUnit(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
		{
			// std::cout << "ManagerMediator: sending CatchUnits request to UnitRoleManager..." << std::endl;
			return ManagerRequest<int, const ::sc2::Unit*>
				(
					aeolusbot,
					constants::ManagerName::UNIT_ROLE_MANAGER,
					constants::ManagerRequestType::CATCH_UNIT,
					unit
				);
		}

		int CalculateMineralGatheringPoints(
			AeolusBot& aeolusbot,
			std::vector<::sc2::Point3D> expansion_locations
		)
		{
			// std::cout << "ManagerMediator: sending CalculateMineralGatheringPoints request to ResourceManager..." << std::endl;
			return ManagerRequest<int, std::vector<::sc2::Point3D>>
				(
					aeolusbot,
					constants::ManagerName::RESOURCE_MANAGER,
					constants::ManagerRequestType::CALCULATE_MINERAL_GATHERING_POINTS,
					expansion_locations
				);
		}

		int AssignInitialWorkers(
			AeolusBot& aeolusbot
		)
		{
			// std::cout << "ManagerMediator: sending AssignInitialWorkers request to ResourceManager..." << std::endl;
			return ManagerRequest<int, int>
				(
					aeolusbot,
					constants::ManagerName::RESOURCE_MANAGER,
					constants::ManagerRequestType::ASSIGN_INITIAL_WORKERS,
					0
				);
		}

		std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*> GetWorkersToPatch(AeolusBot& aeolusbot)
		{
			return ManagerRequest<std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*>>(
				aeolusbot,
				constants::ManagerName::RESOURCE_MANAGER,
				constants::ManagerRequestType::GET_WORKERS_TO_PATCH
			);
		}

		std::unordered_map<const ::sc2::Unit*, ::sc2::Point2D> GetMineralGatheringPoints(AeolusBot& aeolusbot)
		{
			return ManagerRequest<std::unordered_map<const ::sc2::Unit*, ::sc2::Point2D>>(
				aeolusbot,
				constants::ManagerName::RESOURCE_MANAGER,
				constants::ManagerRequestType::GET_MINERAL_GATHERING_POINTS
			);
		}

		::sc2::Units GetAllDestructables(AeolusBot& aeolusbot)
		{
			std::cout << "trying to send get all desrtuctables to neutralunitmanager" << std::endl;
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::NEUTRAL_UNIT_MANAGER,
				constants::ManagerRequestType::GET_ALL_DESTRUCTABLES,
				0
			);
		}

		::sc2::Units GetAllMineralPatches(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::NEUTRAL_UNIT_MANAGER,
				constants::ManagerRequestType::GET_ALL_MINERAL_PATCHES,
				0
			);
		}

		::sc2::Units GetAllVespeneGeysers(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::NEUTRAL_UNIT_MANAGER,
				constants::ManagerRequestType::GET_ALL_VESPENE_GEYSERS,
				0
			);
		}

		::sc2::Units GetAllStructures(AeolusBot& aeolusbot, ::sc2::Unit::Alliance alliance)
		{
			return ManagerRequest<::sc2::Units, ::sc2::Unit::Alliance>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_STRUCTURES,
				alliance
			);
		}

		::sc2::ImageData GetDefaultGridData(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::ImageData, int>(
				aeolusbot,
				constants::ManagerName::PATH_MANAGER,
				constants::ManagerRequestType::GET_DEFAULT_GRID_DATA,
				0
			);
		}



		// populate the managers! 
		// void Populate(AeolusBot&);
	};
}

