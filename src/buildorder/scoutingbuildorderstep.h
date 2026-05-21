#pragma once

#include "buildorderstep.h"
#include <vector>

namespace Aeolus
{
	class AeolusBot;

	/**
	* Sends out a probe to scout the enemy map. This build order step only assigns the scouting role to the
	* probe. The probe will remain with the scouting role until remain in that role for now
	*/
	class ScoutingBuildOrderStep : public BuildOrderStep
	{
	public:
		/**
		* Create a build order step in charge of scouting the targeted base locations
		*/
		ScoutingBuildOrderStep(int supply_threshold, bool scout_enemy_base, bool scout_own_half_of_map, bool scout_enemy_base_first);

		/**
		* @brief Returns the supply threshold of this build order.
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
		bool m_scout_enemy_base; // whether we want to scout 
		bool m_scout_own_half_of_map; // 
		bool m_scout_enemy_base_first;
		bool m_started;
	};
}