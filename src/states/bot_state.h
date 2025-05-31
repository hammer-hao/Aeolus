#pragma once

#include <memory>

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
		virtual void OnEnter() {}

		// Called once per step, performs macro and registers corresponding behavior to the bot object
		virtual void macro(AeolusBot& aeolusbot) = 0;

		// Called once per step, micros forces and registers corresponding micro maneuvers
		virtual void micro(AeolusBot& aeolusbot) = 0;

		// Called once when transitioned out
		virtual void OnExit() {}
	};

	// Helper to construct states
	template<typename T, typename... Args>
	std::unique_ptr<BotState> MakeState(Args&&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}