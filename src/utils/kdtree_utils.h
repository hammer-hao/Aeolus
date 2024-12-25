#pragma once

#include <vector>
#include "../thirdparty/nanoflann.hpp"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <memory>

namespace Aeolus
{
	struct UnitPointCloud
	{
		std::vector<::sc2::Point2D> points;

		// return the number of points
		inline size_t kdtree_get_point_count() const;

		// return the x or y coordinate of the ith point
		inline float kdtree_get_pt(const unsigned long idx, const unsigned long dim) const;

		// Optional: Bounding-box computation
		template <class BBOX> bool kdtree_get_bbox(BBOX&) const;
	};

	using KDTree = nanoflann::KDTreeSingleIndexAdaptor<
		nanoflann::L2_Simple_Adaptor<float, UnitPointCloud>,
		UnitPointCloud,
		2>;

	struct UnitsKDTree
	{
		std::unique_ptr<KDTree> tree;
		std::vector<const ::sc2::Unit*> unit_map;

		static std::unique_ptr<UnitsKDTree> create(const std::vector<const sc2::Unit*>& units);
	};
}