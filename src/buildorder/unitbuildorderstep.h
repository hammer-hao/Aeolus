#pragma once

#include "buildorderstep.h"
#include "sc2api/sc2_typeenums.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief A build order step dedicated to producing a single unit from a
	* given UNIT_ID.
	*/
	class UnitBuildOrderStep : public BuildOrderStep
	{
	public:
		/**
		* @brief Creates a new unit build order for the Aeolus Bot agent given
		* the supply threshold for the creation of this unit and the id of this unit.
		*/
		UnitBuildOrderStep(int supply_threshold, ::sc2::UNIT_TYPEID);

		/**
		* @brief Executes the upgrade build order step. Returns true on when a structure
		* is available to build the unit and the build command is successfully issued. Returns
		* false when otherwise.
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

		/**
		* @brief get whether this build order step is finished.
		*/
		bool isDone(AeolusBot& aeolusbot) override;

		// get whether this build has been started
		bool started() override;

	private:
		int m_supply_threshold;
		::sc2::UNIT_TYPEID m_to_train;
		bool m_started;
	};
}