#pragma once

#include "../contingency.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief The worker rush response state. We enter this state when
	* we see an abnormally high number of enemy workers very early in the game.
	*/
	class WorkerRush : public ContingencyState
	{
	public:
		WorkerRush() {}

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override {}

	private:
		const std::map<::sc2::UNIT_TYPEID, float> m_army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_ZEALOT, 1.0} };
	};
}