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
	* @brief The twelve pool response state. Entered when we observe
	* a spawning pool that is definitely built within ~20 seconds of the
	* start of the game.
	*/
	class TwelvePool : public ContingencyState
	{
	public:
		TwelvePool() {};

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		bool m_rebuild_gateway = false;
		bool m_rebuild_cybercore = false;
		bool m_battery_destroyed = false;
	};
}