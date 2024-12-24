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

		// UnitFilterManager

		::sc2::Units GetAllDestructables(AeolusBot& aeolusbot)
		{
			std::cout << "trying to send get all destructables to unitfiltermanager" << std::endl;
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_DESTRUCTABLES,
				0
			);
		}

		::sc2::Units GetOwnWorkers(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_OWN_WORKERS,
				0
			);
		}

		::sc2::Units GetOwnTownHalls(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_OWN_TOWN_HALLS,
				0
			);
		}

		::sc2::Units GetAllOwnUnits(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_OWN_UNITS,
				0
			);
		}

		::sc2::Units GetAllMineralPatches(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_MINERAL_PATCHES,
				0
			);
		}

		::sc2::Units GetAllVespeneGeysers(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_VESPENE_GEYSERS,
				0
			);
		}

		::sc2::Units GetAllStructures(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_STRUCTURES,
				0
			);
		}

		::sc2::Units GetAllEnemyUnits(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_ENEMY_UNITS,
				0
			);
		}


		// PathManager

		::sc2::ImageData GetDefaultGridData(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::ImageData, int>(
				aeolusbot,
				constants::ManagerName::PATH_MANAGER,
				constants::ManagerRequestType::GET_DEFAULT_GRID_DATA,
				0
			);
		}

		::sc2::Point2D FindClosestGroundSafeSpot(AeolusBot& aeolusbot, ::sc2::Point2D position, double radius)
		{
			return ManagerRequest<::sc2::Point2D, ::sc2::Point2D, double>(
				aeolusbot,
				constants::ManagerName::PATH_MANAGER,
				constants::ManagerRequestType::FIND_CLOSEST_GROUND_SAFE_SPOT,
				position,
				radius
			);
		}

		bool IsGroundPositionSafe(AeolusBot& aeolusbot, ::sc2::Point2D position)
		{
			return ManagerRequest<bool, ::sc2::Point2D>(
				aeolusbot,
				constants::ManagerName::PATH_MANAGER,
				constants::ManagerRequestType::IS_GROUND_POSITION_SAFE,
				position
			);
		}

		// UnitPropertyManager

		bool CanAttackGround(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
		{
			return ManagerRequest<bool, const ::sc2::Unit*>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::CAN_ATTACK_GROUND,
				unit
			);
		}
		bool CanAttackAir(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
		{
			return ManagerRequest<bool, const ::sc2::Unit*>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::CAN_ATTACK_AIR,
				unit
			);
		}
		double GroundRange(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
		{
			return ManagerRequest<double, const ::sc2::Unit*>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::GROUND_RANGE,
				unit
			);
		}
		double AirRange(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
		{
			return ManagerRequest<double, const ::sc2::Unit*>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::AIR_RANGE,
				unit
			);
		}
		double GroundDPS(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
		{
			return ManagerRequest<double, const ::sc2::Unit*>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::GROUND_DPS,
				unit
			);
		}
		double AirDPS(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
		{
			return ManagerRequest<double, const ::sc2::Unit*>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::AIR_DPS,
				unit
			);
		}

		// DefenseManager

		::sc2::Units GetUnitsInRange(AeolusBot& aeolusbot, ::sc2::Units starting_points, float distance)
		{
			return ManagerRequest<::sc2::Units, ::sc2::Units, float>(
				aeolusbot,
				constants::ManagerName::DEFENSE_MANAGER,
				constants::ManagerRequestType::GET_UNITS_IN_RANGE,
				starting_points,
				distance
			);
		}

		::sc2::Units GetGroundThreatsNearBases(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::DEFENSE_MANAGER,
				constants::ManagerRequestType::GET_GROUND_THREATS_NEAR_BASES,
				0
			);
		}

		// populate the managers! 
		// void Populate(AeolusBot&);
	};
}

