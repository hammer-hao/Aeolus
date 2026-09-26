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
		ContingencyState() {}
		std::string_view getName() const override;
		static bool ensureContingencyResponse(AeolusBot& aeolusbot);

	protected:
		static bool ensureResponseAgainstProtoss(AeolusBot& aeolusbot);
		static bool ensureResponseAgainstTerran(AeolusBot& aeolusbot);
		static bool ensureResponseAgainstZerg(AeolusBot& aeolusbot);

		static void sendChatTag(AeolusBot& aeolusbot, std::string to_send);
		bool m_scv_killer_queued = false;
	};
}