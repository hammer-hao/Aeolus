#pragma once

#include "macro_behavior.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief Macro behavior that will issue commands for all SCOUTING units to
	* move to their desinated scout waypoints.
	*/
	class Scout : public MacroBehavior
	{
	public:
		Scout() {};
		~Scout() override = default;

		bool execute(AeolusBot& aeolusbot) override;
	};
}