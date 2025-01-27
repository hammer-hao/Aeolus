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
	class PathToTarget : public MicroManeuver
	{
	public:
		PathToTarget(::sc2::Point2D target) : m_target(target) {}

		~PathToTarget() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		::sc2::Point2D m_target;
	};
}