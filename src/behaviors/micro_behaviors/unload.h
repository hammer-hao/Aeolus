#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief simple behavior for unloading all cargo in a dropship
	*/
	class Unload : public MicroManeuver
	{
	public:
		Unload() {}

		~Unload() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;
	};
}

