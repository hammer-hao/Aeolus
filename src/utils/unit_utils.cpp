#include "unit_utils.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <algorithm>
#include <cmath>


namespace Aeolus
{
	namespace utils {
		::sc2::Point2D ConvertTo2D(const ::sc2::Point3D& point3D)
		{
			return ::sc2::Point2D(point3D.x, point3D.y);
		}

		std::vector<sc2::Point2D> ConvertTo2DVector(const std::vector<::sc2::Point3D> vector_3d)
		{
			std::vector<::sc2::Point2D> vector_2d;
			vector_2d.reserve(vector_3d.size());

			for (const auto& point : vector_3d) {
				vector_2d.emplace_back(point.x, point.y);
			}

			return vector_2d;
		}

		// sort units by distance to a given point, returns sorted ::sc2::Units vector
		::sc2::Units SortByDistanceTo(
			const ::sc2::Units& units,
			const ::sc2::Point2D& target,
			bool reverse)
		{

			std::cout << "Sorting units by distance to target..." << std::endl;
			// copy the vector to avoid modifying the original
			::sc2::Units sorted_units = units;

			// sort based on squared distance to target
			std::sort(sorted_units.begin(), sorted_units.end(),
				[&target, reverse](const ::sc2::Unit* a, const ::sc2::Unit* b)
				{
					if (reverse)
					{
						return ::sc2::Distance2D(ConvertTo2D(a->pos), target) > ::sc2::Distance2D(ConvertTo2D(b->pos), target);
					}
					else
					{
						return ::sc2::Distance2D(ConvertTo2D(a->pos), target) < ::sc2::Distance2D(ConvertTo2D(b->pos), target);
					}
				});

			return sorted_units;
		}

		bool HasAbilityQueued(const sc2::Unit* unit, sc2::ABILITY_ID ability) {
			if (unit == nullptr) {
				return false;
			}

			for (const auto& order : unit->orders) {
				if (order.ability_id == ability) {
					return true;
				}
			}
			return false;
		}
	}
}