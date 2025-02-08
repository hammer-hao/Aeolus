#pragma once

#include "manager.h"
#include "../pathing/grid.h"
#include <sc2api/sc2_common.h>
#include <Eigen/Dense>

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

		std::vector<::sc2::Point2D> _calculateProtossRampPylonPos(::sc2::Point2D main_location,
			const Grid& pathing_grid, const Grid& placement_grid);

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
			const Grid& placement_grid,
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