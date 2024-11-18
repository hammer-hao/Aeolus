#pragma once

#include <iostream>

namespace Aeolus {
	namespace constants
	{
		enum class ManagerName
		{
			UNIT_ROLE_MANAGER,
			RESOURCE_MANAGER
		};

		enum class ManagerRequestType
		{
			GET_UNITS_FROM_ROLE,
			CATCH_UNIT,
			GET_ALL_MINERAL_PATCHES,
			ASSIGN_WORKER_TO_PATCH,
			CLEAR_ASSIGNMENT,
			CALCULATE_MINERAL_GATHERING_POINTS,
			GET_MINERAL_GATHERING_POINTS,
			GET_WORKERS_TO_PATCH,
			ASSIGN_INITIAL_WORKERS
		};

		enum class UnitRole
		{
			GATHERING
		};
		std::string ManagerNameToString(ManagerName name);
		std::string ManagerRequestTypeToString(ManagerRequestType requestType);
	
		const double MINING_RADIUS{ 1.35 };
		const double TOWNHALL_DISTANCE_FACTOR{ 1.08 };
		const double MINING_BOOST_MIN_RADIUS{ 0.5625 };
		const double MINING_BOOST_MAX_RADIUS{ 4.0 };
		const double TOWNHALL_RADIUS{ 2.75 };

		const float CAMERA_WIDTH{ 24.0f };
		const int FEATURE_LAYER_SIZE{ 3 };
		const int PIXEL_DRAW_SIZE{ 180 };
		constexpr int DRAW_SIZE = FEATURE_LAYER_SIZE * PIXEL_DRAW_SIZE;
	}
}