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
		/**
		* @brief creates a new AutoSupply behavior at base index.
		* @param base_location: Index of the base. Main base = 0.
		*/
		AutoSupply(int base_location = 0) : m_base_location(base_location)
		{
		}
		~AutoSupply() override = default;

		bool execute(AeolusBot& aeolusbot) override;
	private:
		int m_base_location;

		/**
		* @brief Calculates the supply required at any given moment.
		* @param aeolusbot: The bot to calculate required supply for.
		*/
		static int _numSupplyRequired(AeolusBot& aeolusbot);
	};
}