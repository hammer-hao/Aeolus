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
		BUILDING_5X5
	};

	using BuildingMap = std::map<std::pair<float, float>, BuildingAttributes>;
	using BuildingTypeMap = std::unordered_map<BuildingTypes, BuildingMap>;
	using ExpansionMap = std::vector<BuildingTypeMap>;

	class PlacementManager :public Manager
	{
	public:
		PlacementManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
			m_expansion_map.resize(16);
		}

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

		std::vector<::sc2::Point2D> _findExansionLocations(const ::sc2::HeightMap& height_map, const Grid& placement_grid);

		::sc2::Point2D _calculateProtossRampPylonPos(::sc2::Point2D main_location,
			const Grid& pathing_grid, const Grid& placement_grid, const ::sc2::HeightMap& height_map);
	};
}