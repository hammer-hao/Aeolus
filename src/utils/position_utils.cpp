#include "position_utils.h"
#include "unit_utils.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_map_info.h>
#include <vector>
#include <limits>
#include <cmath>
#include <iostream>

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
		std::vector<::sc2::Point2D> circleIntersection(const ::sc2::Point2D& c1, const ::sc2::Point2D& c2, double r) {
			std::vector<::sc2::Point2D> result;

			float d = ::sc2::Distance2D(c1, c2);

			// If centers are the same and radii are the same, infinite intersections (coincident circles):
			// For this scenario, we won't handle "infinite" solutions—just return empty or decide how you want to handle it.
			if (d == 0.0) {
				// Could return empty or special-case
				return result;
			}

			// If distance between centers > sum of radii, no intersection
			// Or if one circle is completely inside the other (d < |r - r| == 0) => no intersection
			if (d > 2 * r || d < 0.0) {
				return result;  // no real intersection
			}

			// Otherwise, we find the two intersection points
			// Reference formula for intersecting two circles of equal radius:
			//   1) Find the midpoint M between c1 and c2 along the line connecting them:
			//      M = c1 + (d/2) * ( (c2 - c1) / d )
			//   2) Distance from M to either intersection point = h = sqrt(r^2 - (d/2)^2)
			//   3) Intersection points = M +/- h * ( (c2 - c1) / d ) rotated 90 degrees

			float a = d / 2.0;
			float h = std::sqrt(r * r - a * a);

			// Vector from c1 to c2
			float cx = c2.x - c1.x;
			float cy = c2.y - c1.y;

			// Midpoint M
			float mx = c1.x + (a / d) * cx;
			float my = c1.y + (a / d) * cy;

			// Now offset +/- h in the orthonormal direction
			// Rotate (cx, cy) by 90 deg => ( -cy, cx ) or ( cy, -cx )
			float rx = -cy * (h / d);
			float ry = cx * (h / d);

			// Intersection points
			::sc2::Point2D p1{ mx + rx, my + ry };
			::sc2::Point2D p2{ mx - rx, my - ry };

			if (d == 2 * r) {
				// The two circles touch at exactly one point (p1 == p2)
				result.push_back(p1);
			}
			else {
				// They intersect in two distinct points
				result.push_back(p1);
				result.push_back(p2);
			}

			return result;
		}


		// given a point and a vector of points, get the closest point out of the vector to the given point
		::sc2::Point2D GetClosestTo(::sc2::Point2D x, std::vector<::sc2::Point2D> y)
		{
			float min_distance{ std::numeric_limits<float>::max()};
			::sc2::Point2D closest_point;

			for (auto& point : y)
			{
				float distance = ::sc2::Distance2D(x, point);
				if (distance < min_distance)
				{
					min_distance = distance;
					closest_point = point;
				}
			}
			return closest_point;
		}

		const ::sc2::Unit* GetClosestUnitTo(::sc2::Point2D x, ::sc2::Units y)
		{
			float min_distance{ std::numeric_limits<float>::max() };
			const ::sc2::Unit* closest_unit;

			for (auto& unit : y)
			{
				float distance = ::sc2::Distance2D(x, ConvertTo2D(unit->pos));
				if (distance < min_distance)
				{
					min_distance = distance;
					closest_unit = unit;
				}
			}
			return closest_unit;
		}

		// given an original point and target point, get a position that is a specified distance closer to the target
		::sc2::Point2D GetPositionTowards(::sc2::Point2D x, ::sc2::Point2D y, float distance, bool limit)
		{
			float actual_distance = ::sc2::Distance2D(x, y);
			if (limit && distance > actual_distance)
			{
				distance = actual_distance;
			}

			float delta_x{ y.x - x.x };
			float delta_y{ y.y - x.y };

			// Use Euclidean distance to correctly scale the direction vector
			float scale = distance / actual_distance;
			float new_x = x.x + delta_x * scale;
			float new_y = x.y + delta_y * scale;

			return ::sc2::Point2D(new_x, new_y);
		}

		/*
		given a vector of 2D positions, get a filtered vector of
		the given vector that is closer than a given unit to the target
		*/
		::sc2::Units GetCloserThan(
			::sc2::Units units,
			const float closer_than,
			const ::sc2::Point2D& target
		)
		{
			// create empty vector to populate
			::sc2::Units filtered_units;

			for (const auto& unit : units)
			{
				float distance = ::sc2::Distance2D(ConvertTo2D(unit->pos), target);

				if (distance < closer_than)
				{
					filtered_units.push_back(unit);
				}
			}

			return filtered_units;
		}

		/*
		Given two circles on the map, return two points as their intersections
		*/
		std::vector<::sc2::Point2D> GetCircleIntersection(
			const ::sc2::Point2D& center_x,
			const ::sc2::Point2D& center_y,
			const float& radius_x,
			const float& radius_y
		)
		{
			::std::vector<::sc2::Point2D> intersections;

			float distance{ static_cast<float>(std::sqrt(
				std::pow((center_x.x - center_y.x), 2)
				+ std::pow((center_x.y - center_y.y), 2))) };

			if ((distance > radius_x + radius_y) || (distance < std::abs(radius_x - radius_y)) || distance == 0) 
				return intersections;

			// *a* is the distance a from the center of the first circle to the line connecting the two intersection points.
			float a{ static_cast<float>((std::pow(radius_x, 2) - std::pow(radius_y, 2) + std::pow(distance, 2)) / (2 * distance)) };
				
			// Point P2 is on the line connecting the centers of the two circles. 
			// The following formulas calculate the coordinates (x2, y2) of P2
			float x2{ center_x.x + a * (center_y.x - center_x.x) / distance };
			float y2{ center_x.y + a * (center_y.y - center_x.y) / distance };

			// h is the distance from P2 to each of the intersection points.
			float h{ static_cast<float>(std::sqrt(std::pow(radius_x, 2) - std::pow(a, 2))) };

			// These formulas calculate the coordinates of the two intersection points.
			float a_x{ x2 + h * (center_y.y - center_x.y) / distance };
			float a_y{ y2 - h * (center_y.x - center_x.x) / distance };

			float b_x{ x2 - h * (center_y.y - center_x.y) / distance };
			float b_y{ y2 + h * (center_y.x - center_x.x) / distance };

			intersections.push_back(::sc2::Point2D(a_x, a_y));
			intersections.push_back(::sc2::Point2D(b_x, b_y));

			return intersections;
		}

		std::tuple<float, float, float, float> GetBoundingBox(const std::vector<::sc2::Point2D>& points)
		{
			float x_min = 9999.0f;
			float x_max = 0.0f;
			float y_min = 9999.0f;
			float y_max = 0.0f;

			for (const auto& point : points)
			{
				float x = point.x;
				float y = point.y;
				if (x < x_min) x_min = x;
				if (x > x_max) x_max = x;
				if (y < y_min) y_min = y;
				if (y > y_max) y_max = y;
			}
			return { x_min, x_max, y_min, y_max };
		}

		bool isPowered(::sc2::Point2D position, ::sc2::Units pylons, const ::sc2::HeightMap& terrain_height, float pylon_build_progress)
		{
			// std::cout << "checking if " << position.x << position.y << " is powered..." << std::endl;
			float pylon_power_distancesq = 42.25;
			float pos_height = terrain_height.TerrainHeight(position) - 1.0f;
			for (const auto& pylon : pylons)
			{
				//std::cout << "Pylon height: " << pylon->pos.z << " Position height: " << pos_height << std::endl;
				//std::cout << "Pylon build progress: " << pylon->build_progress << " minumum build progress: " << pylon_build_progress << std::endl;
				//std::cout << "Distance to pylon: " << ::sc2::DistanceSquared2D(position, pylon->pos) << " Mininum required: " << pylon_power_distancesq << std::endl;
				if (pylon->pos.z >= pos_height && pylon->build_progress >= pylon_build_progress
					&& ::sc2::DistanceSquared2D(position, pylon->pos) < pylon_power_distancesq) return true;
			}
			//std::cout << "not powered. " << std::endl;
			return false;
		}

		bool canPlaceStructure(int start_x, int start_y, int building_size, ::sc2::PlacementGrid& placement_grid)
		{
			for (int i = 0; i < building_size; ++i)
			{
				for (int j = 0; j < building_size; ++j)
				{
					if (!placement_grid.IsPlacable({ start_x + i, start_y + j })) return false;
				}
			}
			return true;
		}
	}
}