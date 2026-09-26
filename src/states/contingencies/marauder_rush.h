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
	* @brief The Marauder Rush state. Entered when we detect a marauder
	* near our base before the three minute mark
	*/
	class MarauderRush : public ContingencyState
	{
	public:
		MarauderRush() {};

		void OnEnter(AeolusBot& aeolusbot) override;

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override {}
	};
}