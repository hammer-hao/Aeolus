#pragma once

#include <iostream>
#include <any>
#include <map>
#include <optional>

#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_map_info.h>

#include "../constants.h"
#include "manager.h"
#include "../pathing/grid.h"

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

		int AssignRole(AeolusBot& aeolusbot, const::sc2::Unit* unit, constants::UnitRole unit_role)
		{
			return ManagerRequest<int, const ::sc2::Unit*, constants::UnitRole>(
				aeolusbot,
				constants::ManagerName::UNIT_ROLE_MANAGER,
				constants::ManagerRequestType::ASSIGN_ROLE,
				unit,
				unit_role
			);
		}

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

		std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*> GetWorkersToGeyser(AeolusBot& aeolusbot)
		{
			return ManagerRequest<std::unordered_map<const ::sc2::Unit*, const ::sc2::Unit*>>(
				aeolusbot,
				constants::ManagerName::RESOURCE_MANAGER,
				constants::ManagerRequestType::GET_WORKERS_TO_GEYSER
			);
		}

		std::map<std::pair<float, float>, ::sc2::Point2D> GetMineralGatheringPoints(AeolusBot& aeolusbot)
		{
			return ManagerRequest<std::map<std::pair<float, float>, ::sc2::Point2D>>(
				aeolusbot,
				constants::ManagerName::RESOURCE_MANAGER,
				constants::ManagerRequestType::GET_MINERAL_GATHERING_POINTS
			);
		}

		std::optional<const ::sc2::Unit*> SelectWorkerClosestTo(AeolusBot& aeolusbot, ::sc2::Point2D target_location)
		{
			return ManagerRequest<std::optional<const ::sc2::Unit*>, ::sc2::Point2D>(
				aeolusbot,
				constants::ManagerName::RESOURCE_MANAGER,
				constants::ManagerRequestType::SELECT_WORKER_TO_TARGET,
				target_location
			);
		}

		/**
		* @brief Clears the worker assignment for MINERALS ONLY.
		*/
		int ClearWorkerAssignment(AeolusBot& aeolusbot, const ::sc2::Unit* worker)
		{
			return ManagerRequest<int, const ::sc2::Unit* >(
				aeolusbot,
				constants::ManagerName::RESOURCE_MANAGER,
				constants::ManagerRequestType::CLEAR_ASSIGNMENT,
				worker
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

		::sc2::Units GetOwnGasBuildings(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_OWN_GAS_BUILDINGS,
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

		::sc2::Units GetAllOwnStructures(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_OWN_STRUCTURES,
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

		::sc2::Units GetAllResources(AeolusBot& aeolusbot)
		{
			return ManagerRequest<::sc2::Units, int>(
				aeolusbot,
				constants::ManagerName::UNIT_FILTER_MANAGER,
				constants::ManagerRequestType::GET_ALL_RESOURCES,
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

		Grid GetAstarGrid(AeolusBot& aeolusbot)
		{
			return ManagerRequest<Grid, int>(
				aeolusbot,
				constants::ManagerName::PATH_MANAGER,
				constants::ManagerRequestType::GET_ASTAR_GRID_DATA,
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

		std::vector<::sc2::Point2D> GetFloodFillArea(AeolusBot& aeolusbot, ::sc2::Point2D starting_point, int max_distance)
		{
			return ManagerRequest<std::vector<::sc2::Point2D>, ::sc2::Point2D, int>(
				aeolusbot,
				constants::ManagerName::PATH_MANAGER,
				constants::ManagerRequestType::GET_FLOOD_FILL_AREA,
				starting_point,
				max_distance
			);
		}

		::sc2::Point2D FindNextPathingPoint(AeolusBot& aeolusbot, ::sc2::Point2D start,
			::sc2::Point2D goal, bool sense_danger = true, int danger_distance = 20,
			float danger_threshold = 5.0f, bool smoothing = false, int sensitivity = 3)
		{
			return ManagerRequest<::sc2::Point2D, ::sc2::Point2D, ::sc2::Point2D, bool, int, float, bool, int>(
				aeolusbot,
				constants::ManagerName::PATH_MANAGER,
				constants::ManagerRequestType::GET_NEXT_PATH_POINT,
				start,
				goal,
				sense_danger,
				danger_distance,
				danger_threshold,
				smoothing,
				sensitivity
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

		::sc2::Units GetUnitsInAtttackRange(AeolusBot& aeolusbot, const ::sc2::Unit* unit, ::sc2::Units targets)
		{
			return ManagerRequest<::sc2::Units, const ::sc2::Unit*, ::sc2::Units>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::UNITS_IN_ATTACK_RANGE,
				unit,
				targets
			);
		}

		::sc2::ABILITY_ID GetCreationAbility(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID unit_id)
		{
			return ManagerRequest<::sc2::ABILITY_ID, ::sc2::UNIT_TYPEID>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::GET_CREATION_ABILITY,
				unit_id
			);
		}

		std::pair<int, int> GetUnitCost(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID unit_type)
		{
			return ManagerRequest<std::pair<int, int>, ::sc2::UNIT_TYPEID>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::GET_UNIT_COST,
				unit_type
			);
		}

		::sc2::UNIT_TYPEID GetRequiredTech(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID unit_type)
		{
			return ManagerRequest<::sc2::UNIT_TYPEID, ::sc2::UNIT_TYPEID>(
				aeolusbot,
				constants::ManagerName::UNIT_PROPERTY_MANAGER,
				constants::ManagerRequestType::GET_REQUIRED_TECH,
				unit_type
			);
		}

		int GetUnitSupplyCost(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID unit_type)
		{
			return ManagerRequest<int, ::sc2::UNIT_TYPEID>
				(
					aeolusbot,
					constants::ManagerName::UNIT_PROPERTY_MANAGER,
					constants::ManagerRequestType::GET_UNIT_SUPPLY_COST,
					unit_type
				);
		}

		float GetUnitMovementSpeed(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID unit_type)
		{
			return ManagerRequest<float, ::sc2::UNIT_TYPEID>
				(
					aeolusbot,
					constants::ManagerName::UNIT_PROPERTY_MANAGER,
					constants::ManagerRequestType::GET_UNIT_MOVEMENT_SPEED,
					unit_type
				);
		}

		// DefenseManager

		::sc2::Units GetUnitsInRange(AeolusBot& aeolusbot, std::vector<::sc2::Point2D> starting_points, float distance)
		{
			return ManagerRequest<::sc2::Units, std::vector<::sc2::Point2D>, float>(
				aeolusbot,
				constants::ManagerName::DEFENSE_MANAGER,
				constants::ManagerRequestType::GET_UNITS_IN_RANGE,
				starting_points,
				distance
			);
		}

		::sc2::Units GetOwnUnitsInRange(AeolusBot& aeolusbot, std::vector<::sc2::Point2D> starting_points, float distance)
		{
			return ManagerRequest<::sc2::Units, std::vector<::sc2::Point2D>, float>(
				aeolusbot,
				constants::ManagerName::DEFENSE_MANAGER,
				constants::ManagerRequestType::GET_OWN_UNITS_IN_RANGE,
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

		std::vector<::sc2::Units> GetEnemyUnitsInRangeMap(AeolusBot& aeolusbot, std::vector<::sc2::Point2D> starting_points, float distance)
		{
			return ManagerRequest<std::vector<::sc2::Units>, std::vector<::sc2::Point2D>, float>(
				aeolusbot,
				constants::ManagerName::DEFENSE_MANAGER,
				constants::ManagerRequestType::GET_ENEMY_UNITS_IN_RANGE_MAP,
				starting_points,
				distance
			);
		}

		// PlacementManager

		std::vector<::sc2::Point2D> GetExpansionLocations(AeolusBot& aeolusbot)
		{
			return ManagerRequest<std::vector<::sc2::Point2D>, int>(
				aeolusbot,
				constants::ManagerName::PLACEMENT_MANAGER,
				constants::ManagerRequestType::GET_EXPANSION_LOCATIONS,
				0
			);
		}

		/**
		* @brief Requests a building placement at the target position.
		* @param base_number: The index of the base to make the request
		* @param structure_type: The unit id of the requested structure
		* @param is_wall: requesting to be placed as a wall?
		* @param find_alternative: If no placement available, enabling will allow finding placement at other bases
		* @param reserve_placement: reserve the placement in placementmanager?
		* @param within_power_field: Set ture if not building a pylon/nexus
		* @param pylon_build_progress: consider the pylons with progress more than this
		* @param build_close_to: build close to a give location?
		* @param close_to: if build_close_to, the given location
		*/
		std::optional<::sc2::Point2D> RequestBuildingPlacement(AeolusBot& aeolusbot, 
			int base_number, 
			::sc2::UNIT_TYPEID structure_type,
			bool is_wall = false,
			bool find_alternative = true,
			bool reserve_placement = true,
			bool within_power_field = true,
			float pylon_build_progress = 1.0,
			bool build_close_to = false,
			::sc2::Point2D close_to = { 0, 0 })
		{
			return ManagerRequest<std::optional<::sc2::Point2D>, int, ::sc2::UNIT_TYPEID, bool, bool, bool, bool, float, bool, ::sc2::Point2D>(
				aeolusbot,
				constants::ManagerName::PLACEMENT_MANAGER,
				constants::ManagerRequestType::REQUEST_BUILDING_PLACEMENT,
				base_number,
				structure_type,
				is_wall,
				find_alternative,
				reserve_placement,
				within_power_field,
				pylon_build_progress,
				build_close_to,
				close_to
			);
		}

		// BuildingManager

		bool BuildWithSpecificWorker(AeolusBot& aeolusbot,
			const ::sc2::Unit* worker,
			::sc2::UNIT_TYPEID structure_type,
			::sc2::Point2D position,
			bool assign_role = true)
		{
			return ManagerRequest<bool, const ::sc2::Unit*, ::sc2::UNIT_TYPEID, ::sc2::Point2D, bool>
				(
					aeolusbot,
					constants::ManagerName::BUILDING_MANAGER,
					constants::ManagerRequestType::BUILD_WITH_SPECIFIC_WORKER,
					worker,
					structure_type,
					position,
					assign_role
				);
		}

		size_t GetNumberPending(AeolusBot& aeolusbot, ::sc2::UNIT_TYPEID structure_type)
		{
			return ManagerRequest<size_t, ::sc2::UNIT_TYPEID>(
				aeolusbot,
				constants::ManagerName::BUILDING_MANAGER,
				constants::ManagerRequestType::GET_NUMBER_PENDING,
				structure_type
			);
		}

		// populate the managers! 
		// void Populate(AeolusBot&);
	};
}

