#pragma once
#include "macro_behavior.h"

namespace Aeolus
{
	class AeolusBot;
	/**
	* @brief Controls the gas count of the bot.
	*/
	class BuildGeysers : public MacroBehavior
	{
	public:
		/**
		* @brief Creates a new BuildGeysers behavior.
		* @param to_active_count: The number of mining gas geysers we want (excluding mined out geysers)
		* @param max_pending: Maximum number of geysers to build at a time.
		* @param smart_gas: When true, builds geysers automatically above a given threshold of (minerals - gas).
		* @param smart_gas_threshold: Threshold for smart_gas.
		*/
		BuildGeysers(int to_active_count = 8, int max_pending = 2, bool smart_gas = true, int smart_gas_threshold = 250) :
			m_to_active_count(to_active_count), 
			m_max_pending(max_pending),
			m_smart_gas(smart_gas),
			m_smart_gas_threshold(smart_gas_threshold)
		{
		}
		~BuildGeysers() override = default;
		bool execute(AeolusBot& aeolusbot) override;

	private:
		int m_to_active_count;
		int m_max_pending;
		bool m_smart_gas;
		int m_smart_gas_threshold;
	};
}