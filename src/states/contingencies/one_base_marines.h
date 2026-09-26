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
	* @brief The one base marines state is when we do not observe a
	* natural base from the terran opponent by the 3 minute mark, and 
	* we observe more than 4 marines.
	*/
	class OneBaseMarines : public ContingencyState
	{
	public:
		OneBaseMarines() {};

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override {}
	};
}