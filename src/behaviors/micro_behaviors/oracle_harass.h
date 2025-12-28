#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <vector>

namespace Aeolus 
{
	class AeolusBot;

	/**
	* @brief Complete logic for harassing with an Oracle 
	*/
	class OracleHarass : MicroManeuver 
	{
	public:
		OracleHarass(std::vector<::sc2::Point2D> targets) : m_targets(targets)
		{
		}

		~OracleHarass() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		std::vector<::sc2::Point2D> m_targets;
	};
}