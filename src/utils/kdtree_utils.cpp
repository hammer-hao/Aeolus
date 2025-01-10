#include "kdtree_utils.h"
#include <memory>

namespace Aeolus
{
	std::unique_ptr<UnitsKDTree> UnitsKDTree::create(const std::vector<const ::sc2::Unit*>& units)
	{
		if (units.empty())
		{
			// std::cout << "UnitsKDTree::create - No units to build KDTree." << std::endl;
			return std::make_unique<UnitsKDTree>();
		}

		auto units_kd_tree = std::make_unique<UnitsKDTree>();

		for (const auto& unit : units)
		{
			if (unit)
			{
				units_kd_tree->cloud.points.emplace_back(::sc2::Point2D(unit->pos));
				units_kd_tree->unit_map.push_back(unit);
			}
		}

		units_kd_tree->tree = std::make_unique<KDTree>(2, units_kd_tree->cloud, nanoflann::KDTreeSingleIndexAdaptorParams(10));
		units_kd_tree->tree->buildIndex();
		return units_kd_tree;
	}
}