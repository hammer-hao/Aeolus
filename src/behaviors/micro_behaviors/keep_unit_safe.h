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
	class KeepUnitSafe : public MicroManeuver
	{
	public:
		KeepUnitSafe() {}

		KeepUnitSafe(::sc2::Point2D target) : m_target(target)
		{
		}

		~KeepUnitSafe() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		std::optional<::sc2::Point2D> m_target;
	};
}