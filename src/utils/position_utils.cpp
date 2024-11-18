#include "position_utils.h"
#include "unit_utils.h"

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <vector>
#include <limits>
#include <cmath>
#include <iostream>

namespace Aeolus
{
	namespace utils
	{
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
	}
}