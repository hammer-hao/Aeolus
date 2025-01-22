#pragma once

#include "macro_behavior.h"
#include <sc2api/sc2_common.h>

namespace Aeolus
{
	/* 
	* @brief Automatically makes expansions to a specified count.
	*/
	class Expand :public MacroBehavior
	{
	public:
		Expand(int to_count = 8, int max_pending = 1) : m_to_count(to_count), m_max_pending(max_pending)
		{
		}
		~Expand() override = default;
		bool execute(AeolusBot& aeolusbot) override;

	private:
		int m_to_count;
		int m_max_pending;

		::sc2::Point2D _getNextExpansionLocation(AeolusBot& aeolusbot);

		bool _locationIsBlocked(AeolusBot& aeolusbot, ::sc2::Point2D location);
	};
}