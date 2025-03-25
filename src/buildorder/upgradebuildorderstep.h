#pragma once

#include "buildorderstep.h"
#include <sc2api/sc2_data.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief A build orders step dedicated to building an upgrade / research.
	* On execution, will attempt to research the upgrade.
	*/
	class UpgradeBuildOrderStep : public BuildOrderStep
	{
	public:
		/**
		* @brief Creates a new upgrade build order step based on the bot agent, the supply threshold for
		* execution, and the specific ability to upgrade.
		*/
		UpgradeBuildOrderStep(int supply_threshold, ::sc2::UPGRADE_ID to_research);

		/**
		* @brief Executes the upgrade build order step. Returns true on either when the ability is already
		* researching/researched or having successfully started researching the ability. Returns false if
		* the facility is already researching something else / we dont have the facility available.
		*/
		bool execute(AeolusBot& aeolusbot) override;

		/**
		* @brief Returns the supply threshold of this build order step.
		*/
		int getSupplyThreshold() override;

		/**
		* @brief get the string representation of this build order step.
		*/
		std::string_view toString() override;

	private:
		int m_supply_threshold;
		::sc2::UPGRADE_ID m_to_research;
	};
}