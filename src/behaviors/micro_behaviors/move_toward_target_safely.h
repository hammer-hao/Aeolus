#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include "../../utils/unit_utils.h"

namespace Aeolus
{
	class AeolusBot;

	/*
	* @brief Issues a move command towards the closest safe spot from which the unit can
	* attack a target.
	* If given a vector of units, will pick a target before the calculation is done.
	*/
	class MoveTowardTargetSafely : public MicroManeuver
	{
	public:
		MoveTowardTargetSafely(::sc2::Unit* target) : m_target(target) {}

		MoveTowardTargetSafely(::sc2::Units potentialTargets) 
		{
			m_target = utils::PickAttackTarget(potentialTargets);
		}

		~MoveTowardTargetSafely() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		const ::sc2::Unit* m_target;
	};
}