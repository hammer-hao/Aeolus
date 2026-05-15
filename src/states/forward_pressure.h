#pragma once

#include "bot_state.h"
#include "base_state.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief the forward pressure state. In this state. The bot will send armies to pressure the opponent,
	* forcing them to defend, while actively expanding at home to maintain map control and advantage.
	* We want this state if: 1) we have a better economy than the opponent and 2) our trade is not terrible,
	* i.e. not trading 10 stalkers for 1 marine.
	*/
	class ForwardPressureState : public BaseState
	{
	public:
		ForwardPressureState() {}

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override;

	private:
		void _transitionIntoConsolidateIfNeeded(AeolusBot& aeolusbot);
	};
}