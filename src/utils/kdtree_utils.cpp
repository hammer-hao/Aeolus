#include "kdtree_utils.h"
#include <memory>

namespace Aeolus
{
	inline size_t UnitPointCloud::kdtree_get_point_count() const { return points.size(); }

	inline float UnitPointCloud::kdtree_get_pt(const unsigned long idx, const unsigned long dim) const
	{
		return dim == 0 ? points[idx].x : points[idx].y;
	}
	template <class BBOX> bool UnitPointCloud::kdtree_get_bbox(BBOX&) const { return false; }

	std::unique_ptr<UnitsKDTree> UnitsKDTree::create(const std::vector<const ::sc2::Unit*>& units)
	{
		UnitPointCloud cloud;
		auto units_kd_tree = std::make_unique<UnitsKDTree>();

		for (const auto& unit : units)
		{
			if (unit)
			{
				cloud.points.emplace_back(::sc2::Point2D(unit->pos));
				units_kd_tree->unit_map.push_back(unit);
			}
		}

		units_kd_tree->tree = std::make_unique<KDTree>(2, cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
		units_kd_tree->tree->buildIndex();
		return units_kd_tree;
	}
}