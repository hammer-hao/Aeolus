#pragma once

#include <memory>

#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief Abstract base class for a state that our bot is in. Each state implements
	* their own micro and macro strategies
	*/
	class BotState
	{
	public:
		virtual ~BotState() = default;

		// Called once when transitioned in
		virtual void OnEnter(AeolusBot& aeolusbot) {}

		// Called once per step, performs macro and registers corresponding behavior to the bot object
		virtual void macro(AeolusBot& aeolusbot) = 0;

		// Called once per step, micros forces and registers corresponding micro maneuvers
		virtual void micro(AeolusBot& aeolusbot) = 0;

		// Called once when transitioned out
		virtual void OnExit() {}

		// Called once for every unit destroyed. In case a state needs to track units lost
		virtual void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) {}
	};

	// Helper to construct states
	template<typename T, typename... Args>
	std::unique_ptr<BotState> MakeState(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}