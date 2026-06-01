#pragma once

#include "manager.h"
#include <map>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	enum class HarassmentStatus
	{
		HEADING_TO_BASE,
		HARASSING_AT_MAIN,
		HARASSING_AT_NATURAL,
		HARASSING_AT_THIRD,
		SURVIVING,
	};

	class HarassmentManager : public Manager
	{
	public:
		HarassmentManager(AeolusBot& aeolusbot) : m_bot(aeolusbot)
		{
		}

		std::string_view GetName() const override {
			static const std::string name = "HarassmentManager";
			return name;
		}

		std::any ProcessRequest(AeolusBot& aeolusbot, constants::ManagerRequestType request, std::any args) override;

		void HarassmentManager::Initialize();

		void update(int iteration) override;

		void onUnitDestroyed(const ::sc2::Unit* unit);

	private:
		AeolusBot& m_bot;

		std::map<::sc2::Tag, HarassmentStatus> m_harassment_tracker;

		std::vector<std::vector<::sc2::Point2D>> m_position_behind_enemy_main_natural_third;

		std::vector<std::vector<::sc2::Point2D>> _getPositionBehindEnemyMainNaturalThird();

		std::map<::sc2::Tag, HarassmentStatus> _getHarassmentTracker();

		void _registerHarassmentStatus(::sc2::Tag unitTag, HarassmentStatus status);
	};
}