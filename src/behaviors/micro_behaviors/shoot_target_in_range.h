#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;
	/*
	@brief Pathes the unit to the given target, avoiding dangerous zones
	on the map.
	*/
	class ShootTargetInRange : public MicroManeuver
	{
	public:
		ShootTargetInRange(::sc2::Units targets) : m_targets(targets) {}

		~ShootTargetInRange() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

		bool _isAttackReady(AeolusBot& aeolusbot, const ::sc2::Unit* unit, const ::sc2::Unit* target);

	private:
		::sc2::Units m_targets;
	};
}