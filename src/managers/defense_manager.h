#pragma once

#include "manager.h"
#include "../constants.h"
#include "nanoflann.hpp"
#include "../utils/kdtree_utils.h"

#include <vector>
#include <sc2api/sc2_common.h>
#include <memory>

namespace Aeolus
{
	class AeolusBot;

	using StartPointType = std::variant<const sc2::Unit*, ::sc2::Point2D>;

	class DefenseManager : public Manager
	{
	public:
		DefenseManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
			m_ground_threat_range = 15;
		}

		std::string_view GetName() const override {
			static const std::string name = "DefenseManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

		void GenerateKDTrees();

		::sc2::Units UnitsInRange(const std::vector<StartPointType>& starting_points, float distance, UnitsKDTree& query_tree);

		::sc2::Units MainThreatsNearTownHall();

	private:
		AeolusBot& m_bot;

		float m_ground_threat_range;
		::sc2::Units m_own_town_halls;
		std::unique_ptr<UnitsKDTree> m_all_own_units_tree;
		std::unique_ptr<UnitsKDTree> m_all_enemy_units_tree;
	};
}