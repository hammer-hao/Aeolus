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
		UseAbility(::sc2::ABILITY_ID abilityToUse) : m_ability(abilityToUse), m_target(0.0, 0.0), m_has_target(false)
		{
		}

		UseAbility(::sc2::ABILITY_ID abilityToUse, ::sc2::Point2D target) : m_ability(abilityToUse), m_target(target), m_has_target(true)
		{
		}

		~UseAbility() override = default;

		bool execute(AeolusBot& aeolusbot, const ::sc2::Unit*) override;

	private:
		::sc2::ABILITY_ID m_ability;

		::sc2::Point2D m_target;

		bool m_has_target;
	};
}