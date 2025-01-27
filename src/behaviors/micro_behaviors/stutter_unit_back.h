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
	class StutterUnitBack : public MicroManeuver
	{
	public:
		StutterUnitBack(const ::sc2::Unit* target) : m_target(target) {}

		~StutterUnitBack() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		const ::sc2::Unit* m_target;
	};
}