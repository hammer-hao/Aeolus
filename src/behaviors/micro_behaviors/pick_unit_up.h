#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief simple behavior for picking up a target unit
	*/
	class PickUnitUp : public MicroManeuver
	{
	public:
		PickUnitUp(const ::sc2::Unit* target) : m_target(target) {}

		~PickUnitUp() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		const ::sc2::Unit* m_target;
	};
}
