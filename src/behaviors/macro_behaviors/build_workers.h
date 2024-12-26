#pragma once

#include "macro_behavior.h"

namespace Aeolus
{
	class AeolusBot;
	class BuildWorkers :public MacroBehavior
	{
		/* Build Workers behavior, ideally registered during every step
		Executing this behavior will instruct idle nexi to construct probes
		until the desired amount is reached.
		*/
	public:
		BuildWorkers(int to_count = 76) : m_to_count(to_count)
		{
		}
		~BuildWorkers() override = default;
		void execute(AeolusBot& aeolusbot) override;

	private:
		int m_to_count;
	};
}