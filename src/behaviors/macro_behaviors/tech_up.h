#pragma once

#include "macro_behavior.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>
#include <variant>

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief The tech up macro behavior will construct structures in the tech tree
	* until we are ready to build the given unit/upgrade
	*/
	class TechUp : public MacroBehavior
	{
	public:
		TechUp(::sc2::UNIT_TYPEID unitType) : m_target(unitType) {}

		TechUp(::sc2::UPGRADE_ID upgradeType) : m_target(upgradeType) {}

		~TechUp() override = default;

		/**
		* @brief returns true if we are still progressing towards this tech,
		* false if we have finished teching up already.
		*/
		bool execute(AeolusBot& aeolusbot) override;

	private:
		std::variant<::sc2::UNIT_TYPEID, ::sc2::UPGRADE_ID> m_target;
	};
}