#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	/*
	* @brief Issues an Attack command on a target unit, but avoids repeating the command if it is already being performed.
	*/
	class AttackTargetUnit : public MicroManeuver
	{
	public:
		AttackTargetUnit(const ::sc2::Unit* target) : m_target(target) {}

		~AttackTargetUnit() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		const ::sc2::Unit* m_target;
	};
}