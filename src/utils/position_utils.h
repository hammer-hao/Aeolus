#pragma once
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_map_info.h>
#include <vector>

namespace Aeolus
{
	namespace utils
	{
		/**
		 * @brief Returns intersection points of two circles:
		 *        Both circles have radius r.
		 *        The first circle is centered at c1; the second is centered at c2.
		 *
		 * @param c1 Center of the first circle
		 * @param c2 Center of the second circle
		 * @param r  Radius of each circle (both are the same)
		 * @return A vector of 0, 1, or 2 intersection points
		 */
		std::vector<::sc2::Point2D> circleIntersection(const ::sc2::Point2D& c1, const ::sc2::Point2D& c2, double r);

		/**
		 * @brief Returns four values of the bounding box of given positions
		 *
		 * @param points Points to draw the bounding box on
		 * @return A tuple of four floats bounding given points
		 */
		std::tuple<float, float, float, float> GetBoundingBox(const std::vector<::sc2::Point2D>& points);

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

		/*
		Returns whether the position is covered by the power field of a pylon.
		*/
		bool isPowered(::sc2::Point2D position, ::sc2::Units pylons, const ::sc2::HeightMap& terrain_height, float pylon_build_progress = 0.8f);

		/*
		Returns whether the give structure can be placed at the given position
		*/
		bool canPlaceStructure(int start_x, int start_y, int building_size, ::sc2::PlacementGrid& placement_grid);
	}
}