#pragma once
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <vector>

namespace Aeolus
{
	namespace utils
	{
		// given a point and a vector of points, get the closest point out of the vector to the given point
		::sc2::Point2D GetClosestTo(::sc2::Point2D x, std::vector<::sc2::Point2D> y);

		// given a point and Units, get the closest unit out of the units to the given point
		const ::sc2::Unit* GetClosestUnitTo(::sc2::Point2D x, ::sc2::Units y);

		// given an original point and target point, get a position that is a specified distance closer to the target
		::sc2::Point2D GetPositionTowards(::sc2::Point2D x, ::sc2::Point2D y, float distance = 1, bool limit = false);

		/*
		given Units, get a filtered Units that are closer than given distance to a target
		*/
		::sc2::Units GetCloserThan(::sc2::Units units, const float closer_than, const ::sc2::Point2D& target);

		/*
		Given two circles on the map, return two points as their intersections
		*/
		std::vector<::sc2::Point2D> GetCircleIntersection(
			const ::sc2::Point2D& center_x,
			const ::sc2::Point2D& center_y,
			const float& radius_x,
			const float& radius_y
		);
	}
}