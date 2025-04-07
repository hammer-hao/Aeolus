#pragma once

#include "buildorderstep.h"
#include <sc2api/sc2_typeenums.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	 * @brief A build order step dedicated to applying chrono boost to
	 * the specified structure.
	 */
	class ChronoBuildOrderStep : public BuildOrderStep
	{
	public:
		/**
		* @brief creates a new chrono boost build order step, specifying which
		* building type to apply the chrono boost to, and the supply threshold to
		* do so.
		*/
		ChronoBuildOrderStep(int supply_threshold, ::sc2::UNIT_TYPEID to_chrono) :
			m_to_chrono(to_chrono), m_supply_threshold(supply_threshold), m_started(false)
		{}

		/**
		* @brief returns the supply threshold for this build order
		*/
		int getSupplyThreshold() override;

		/**
		* @brief returns the structure name as the string representation of this build
		* order step.
		*/
		std::string_view toString() override;

		/**
		* @brief get whether this build order step is finished.
		*/
		bool isDone(AeolusBot& aeolusbot) override;

		// get whether this build has been started
		bool started() override;

		bool execute(AeolusBot& aeolusbot) override;

	private:
		int m_supply_threshold;
		::sc2::UNIT_TYPEID m_to_chrono;
		bool m_started;
	};
}