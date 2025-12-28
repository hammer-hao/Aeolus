#pragma once

#include "bot_state.h"
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
	class ForwardPressureState : public BotState
	{
	public:
		ForwardPressureState() : m_enteredAt(0) {}

		void OnEnter(AeolusBot& aeolusbot) override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override;

		void OnExit() override;

	private:

		void _micro(AeolusBot& aeolusbot, ::sc2::Units forces, ::sc2::Point2D target);

		void _prismMicro(AeolusBot& aeolusbot, ::sc2::Units prisms, ::sc2::Point2D prismTarget);

		void _oracleHarassMicro(AeolusBot& aeolusbot, ::sc2::Units oracles, std::vector<::sc2::Point2D> harassLocations);

		int m_enteredAt;

		std::deque<std::pair<uint64_t, int>> m_ownLosses;
		std::deque<std::pair<uint64_t, int>> m_opponentLosses;
	};
}