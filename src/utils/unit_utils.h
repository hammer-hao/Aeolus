#pragma once

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

#include <vector>

namespace Aeolus {
	namespace utils {

		// helper function to convert 3D position to 2D
		::sc2::Point2D ConvertTo2D(const ::sc2::Point3D& point3D);

		::std::vector<::sc2::Point2D> ConvertTo2DVector(const std::vector<::sc2::Point3D> vector_3d);

		::sc2::Units SortByDistanceTo(
			const ::sc2::Units& units, 
			const ::sc2::Point2D& target,
			bool reverse = false);

		const ::sc2::Unit* PickAttackTarget(::sc2::Units targets);
	}
}