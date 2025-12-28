#pragma once

#include "bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief the consolidation state. In this state. The bot will seek to preserve as much army as possible
	* and only take defensive fights / winning fights. If we already have a bigger economy, but our army is taking
	* losing / outnumbered fights, this is the state to use until we have built a big enough army to overwelm.
	*/
	class ConsolidateState : public BotState
	{
	public:
		ConsolidateState() = default;

		void OnEnter(AeolusBot& aeolusbot) override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override {}

		void OnExit() override;

	private:
		void _micro(AeolusBot& aeolusbot, ::sc2::Units forces, ::sc2::Point2D target);

		void _prismMicro(AeolusBot& aeolusbot, ::sc2::Units prisms, ::sc2::Point2D prismTarget);

		void _oracleHarassMicro(AeolusBot& aeolusbot, ::sc2::Units oracles, std::vector<::sc2::Point2D> harassLocations);
	};
}