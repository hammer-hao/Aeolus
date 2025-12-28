#pragma once

#include "micro_maneuver.h"
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief Orders an unit to performa an ability (no target)
	*/
	class UseAbility : public MicroManeuver
	{
	public: 
		UseAbility(::sc2::ABILITY_ID abilityToUse) : m_ability(abilityToUse)
		{
		}

		~UseAbility() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit*) override;

	private:
		::sc2::ABILITY_ID m_ability;
	};
}