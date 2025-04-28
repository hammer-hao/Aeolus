#pragma once

#include "manager.h"
#include "../pathing/grid.h"
#include <sc2api/sc2_common.h>
#include <Eigen/Dense>
#include <tuple>

namespace Aeolus
{
	class AeolusBot;

	struct BuildingAttributes {
		bool available = true;
		bool is_wall = false;
		int building_tag = 0;
		bool worker_on_route = false;
		double time_requested = 0.0;
		bool production_pylon = false;
		bool optimal_pylon = false;

		BuildingAttributes(
			bool available = true,
			bool is_wall = false,
			int building_tag = 0,
			bool worker_on_route = false,
			double time_requested = 0.0,
			bool production_pylon = false,
			bool optimal_pylon = false
			) : available(available), is_wall(is_wall), building_tag(building_tag),
			worker_on_route(worker_on_route), time_requested(time_requested),
			production_pylon(production_pylon), optimal_pylon(optimal_pylon)
		{
		}
	};

	enum BuildingTypes
	{
		BUILDING_2X2,
		BUILDING_3X3,
		BUILDING_5X5,
		NOT_FOUND
	};

	using BuildingMap = std::map<std::pair<float, float>, BuildingAttributes>;
	using BuildingTypeMap = std::unordered_map<BuildingTypes, BuildingMap>;
	using ExpansionMap = std::vector<BuildingTypeMap>;

	class PlacementManager :public Manager
	{
	public:

		PlacementManager(AeolusBot& aeolusbot);

		std::string_view GetName() const override
		{
			static const std::string_view name = "PlacementManager";
			return name;
		}

		void update(int iteration) override;

		void Initialize();

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

	private:
		AeolusBot& m_bot;

		ExpansionMap m_expansion_map;

		::sc2::HeightMap m_height_map;

		::sc2::PlacementGrid m_placement_grid;

		std::vector<::sc2::Point2D> m_expansion_locations;

		Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> m_occupied_points;

		void _addPlacementPosition(
			BuildingTypes building_type,
			int expansion_location,
			::sc2::Point2D position,
			bool available = true, 
			bool is_wall = false,
			int building_tag = 0, 
			bool worker_on_route = false,
			double time_requested = 0.0,
			bool production_pylon = false,
			bool optimal_pylon = false);

		std::vector<::sc2::Point2D> _findExansionLocations();

		std::vector<std::vector<std::pair<int, int>>>
			GetRampClusters(const Grid& pathing_grid, const Grid& placement_grid,
				std::size_t min_cluster_size = 8);

		std::vector<::sc2::Point2D> _calculateProtossRampPylonPos(::sc2::Point2D main_location,
			const Grid& pathing_grid, const Grid& placement_grid, const std::vector<std::vector<std::pair<int, int>>>& ramp_clusters);

		void _calculateProtossNaturalWall(std::vector<::sc2::Point2D> expansion_locations, 
			const std::vector<std::vector<std::pair<int, int>>>& ramp_clusters);

		/**
		* @brief returns a vector of m number of (x, y) offsets representing
		* axis around a point.
		*/
		std::vector<std::pair<double, double>> makeSampleAxis(int m);

		/**
		* @brief given a vector of reference points, construct mock axis from
		* the reference points, and identify 1) the ref point at the narrowest
		* choke 2) the optimal axis, and finally the two ends of the ramp
		*/
		std::pair<std::pair<int, int>, std::pair<int, int>> findChoke(std::vector<::sc2::Point2D> refs,
			const ::sc2::PlacementGrid& placementGrid, 
			const ::sc2::HeightMap& heightMap,
			int radius = 15,
			int targetWidth = 11);

		/**
		* @brief alternative to findChoke when the natural choke is also a ramp. Given all ramp clusters,
		* identifies the ramp cluster representing the natural choke, then returns the two (upper) ends
		* of the ramp through processing the top level of this ramp cluster
		*/
		std::pair<std::pair<int, int>, std::pair<int, int>> findChokeByRamp(
			const std::vector<std::vector<std::pair<int, int>>>& ramp_clusters,
			const ::sc2::HeightMap& heightMap,
			const ::sc2::PlacementGrid& placementGrid,
			const ::sc2::PathingGrid& pathingGrid,
			::sc2::Point2D naturalPos
		);

		/**
		* @brief given the two ends of a ramp, the start end and the target end,
		* return the desired 3x3 position next to the start end
		*/
		std::pair<int, int> findNatural3x3Placment(std::pair<int, int> startEnd, std::pair<int, int> targetEnd,
			const ::sc2::PlacementGrid& placementGrid, std::pair<int, int> reference = {0,0});

		/**
		* @brief given the first two placements of the 3x3 wall building at the natural,
		* find the third one which leaves a 1x1 gap
		* @returns 1) the final 3x3 position 2) the position of the "wall" unit to fill the 1x1 gap
		*/
		std::pair<std::pair<int, int>, std::pair<int, int>> findNaturalFinal3x3Placement(std::pair<int, int> pos1,
			std::pair<int, int> pos2, const ::sc2::PlacementGrid& placementGrid, ::sc2::Point2D naturalPos);

		/**
		* @brief given a origin point on a grid, return all points on the middle of cells that are within
		* the given max distance to the origin point
		*/
		std::set<std::pair<int, int>> pointsWithinDistance(std::pair<int, int> center, float maxDistance);

		/**
		* @brief given 1 - 3 3x3 buildings in the wall, calculate the pylon position to power
		* all 3 buildings and is as close to the natural as possible
		*/
		std::pair<int, int> findNaturalPylonPlacement(std::vector <std::pair<int, int>> threeByThrees,
			const ::sc2::PlacementGrid& placementGrid, ::sc2::Point2D naturalPos);

		/**
		* @brief Requests and reserves a placement for a building.
		* @param base_number: The base to request the placement for
		* @param structure_type: The unit id of the structure requested
		* @param is_wall: Is the structure a wall?
		* @param find_alternative: If true, consider other bases if no placement is found
		* @param reserve_placement: Reserve this booking so no other request is made at location
		* @param within_power_field: If true, only find positions within pylon field
		* @param build_close_to: build close to a give position?
		* @param close_to: is build_close_to, the position to build near by
		* @return The building location assigned by PlacementManager for the requested structure
		*/
		std::optional<::sc2::Point2D> _requestBuildingPlacement(int base_number, ::sc2::UNIT_TYPEID structure_type,
			bool is_wall = false,
			bool find_alternative = true,
			bool reserve_placement = true,
			bool within_power_field = true,
			float pylon_build_progress = 0.8f,
			bool build_close_to = false,
			::sc2::Point2D close_to = { 0, 0 });

		std::vector<::sc2::Point2D> _findPotentialPlacementsAtBase(BuildingTypes building_size, int base_index,
			bool within_power_field, float pylon_build_progress = 1.0);

		bool _canPlaceStructure(::sc2::Point2D position, BuildingTypes building_size, bool is_geyser=false);

		static BuildingTypes _structureToBuildingSize(::sc2::UNIT_TYPEID structure_id);

		std::vector<::sc2::Point2D> _findBuildingLocations(
			const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>& kernel,
			std::tuple<float, float, float, float> bounds,
			size_t xStride,
			size_t yStride,
			const ::sc2::PlacementGrid& placement_grid,
			const Grid& pathing_grid,
			const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> occupied_points,
			size_t buildingWidth,
			size_t buildingHeight);

		// Naive 2D "valid" convolution, returning an Eigen::Matrix<uint8_t, Dynamic, Dynamic>
		Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>
			convolve2dValid(
				const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>& input,
				const Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>& kernel);
	};
}