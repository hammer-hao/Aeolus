#pragma once

#include "bot_state.h"
#include "base_state.h"
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
	class BuildOrderState : public BaseState
	{
	public:
		BuildOrderState() {}

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override {}

	private:
		void _oracleHarassMicro(AeolusBot& aeolusbot, ::sc2::Units oracles, std::vector<::sc2::Point2D> harassLocations);
	};
}