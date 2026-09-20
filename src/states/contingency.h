#pragma once

#include "bot_state.h"
#include "base_state.h"
#include "../buildorder/contingency_plan.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief the contingency state.
	*/
	class ContingencyState : public BaseState
	{
	public:
		ContingencyState(const ContingencyPlan& contingencyPlan);

		std::string_view getName() const override;
		
		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

	private:
		void _doSCVKillerMicro(AeolusBot& aeolusbot);
		void _releaseSCVKillers(AeolusBot& aeolusbot);
		const ContingencyPlan m_plan;
		bool m_build_defense_queued;
		bool m_scv_killer_queued = false;
	};
}