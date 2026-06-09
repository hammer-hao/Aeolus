#pragma once

#include "manager.h"
#include "../buildorder/contingency_plan.h"
#include <unordered_map>
#include <sc2api/sc2_common.h>
#include <nlohman/json.hpp>

namespace Aeolus
{
	using json = nlohmann::json;
	class AeolusBot;

	class ConfigManager : public Manager
	{
	public:
		ConfigManager(AeolusBot& aeolusbot);

		std::string_view GetName() const override {
			static const std::string name = "ConfigManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void update(int iteration) override;

	private:
		AeolusBot& m_bot;
		std::unordered_map<::sc2::Race, std::vector<ContingencyPlan>> m_contingency_plans;

		std::optional<::sc2::Point2D> m_natual_pylon_position;
		std::optional<std::vector<::sc2::Point2D>> m_natural_three_by_three_positions;
		std::optional<::sc2::Point2D> m_natural_door_position;

		std::vector<ContingencyPlan> _getContingencyPlans();
		void _loadContingencyPlans(const json& j);
		void _loadNaturalWallPlacements(const json& wall_data);
		::sc2::UNIT_TYPEID _unitNameToType(const std::string& name);

		std::vector<ContingencyPlan> _flattenContingencyPlans(
			const std::unordered_map<
			::sc2::Race,
			std::vector<ContingencyPlan>>&plans);
	};
}