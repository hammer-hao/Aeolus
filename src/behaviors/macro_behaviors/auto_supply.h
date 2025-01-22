#pragma once
#include "macro_behavior.h"

namespace Aeolus
{
	/**
	* @brief Build pylons automatically.
	*/
	class AutoSupply : public MacroBehavior
	{
	public:
		AutoSupply(int base_location = 0) : m_base_location(base_location)
		{
		}
		~AutoSupply() override = default;

		bool execute(AeolusBot& aeolusbot) override;
	private:
		int m_base_location;

		static int _numSupplyRequired(AeolusBot& aeolusbot);
	};
}