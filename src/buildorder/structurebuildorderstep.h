#pragma once

#include <sc2api/sc2_unit.h>

#include "buildorderstep.h"
#include <string>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief A build order step dedicated to building structures. Do not use
	* to make a build order step for unit/upgrades
	*/
	struct StructureBuildOrderStep : public BuildOrderStep
	{
	public:
		/**
		* @brief create a new structurebuildorderstep, requires the supply threshold, the unit typeid of
		* the structure, and whether the structure is a wall.
		*/
		StructureBuildOrderStep(int supply_threshold, ::sc2::UNIT_TYPEID to_build, bool is_wall) :
			m_supply_threshold(supply_threshold), m_to_build(to_build), m_is_wall(is_wall) {}

		/**
		* @brief Returns the supply threshold of this build order.
		*/
		int getSupplyThreshold() override;

		/**
		* @brief returns the structure name as the string representation of this build
		* order step.
		*/
		std::string_view toString() override;

		bool execute(AeolusBot& aeolusbot) override;

	private:
		int m_supply_threshold;
		::sc2::UNIT_TYPEID m_to_build; // the id of the structure we want to build
		bool m_is_wall; // if this structure is a wall
	};
}