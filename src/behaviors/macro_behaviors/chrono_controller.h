#pragma once

#include "macro_behavior.h"


namespace Aeolus
{
	class AeolusBot;

	class ChronoController : public MacroBehavior
	{
	public:
		ChronoController() {}
		~ChronoController() override = default;

		bool execute(AeolusBot& aeolusbot) override;
	};
}