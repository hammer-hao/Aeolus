#pragma once

#include "bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief the build order state. In this state. The bot will attempt to execute the build order step
	* by step, and attack/defend according to the default build order behavior.
	* The bot will exit the build order state when 1) the build order is completed or 2) when we scout
	* something that requires a specific response state.
	*/
	class BuildOrderState : public BotState
	{
	public:
		BuildOrderState() {}

		void OnEnter(AeolusBot& aeolusbot) override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override {}

		void OnExit() override;

	private:

		void _micro(AeolusBot& aeolusbot, ::sc2::Units forces, ::sc2::Point2D target);

		void _prismMicro(AeolusBot& aeolusbot, ::sc2::Units prisms, ::sc2::Point2D prismTarget);
	};
}