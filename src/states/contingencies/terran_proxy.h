#pragma once

#include "../contingency.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

namespace Aeolus
{
	class AeolusBot;

	/**
	* @brief We enter this state upon observing a early terran enemy building
	* constructed on the map, either near our base or away from their base.
	*/
	class TerranProxy : public ContingencyState
	{
	public:
		TerranProxy() {};

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override {}

	private:
		void _doSCVKillerMicro(AeolusBot& aeolusbot);
		void _releaseSCVKillers(AeolusBot& aeolusbot);
		const std::map<::sc2::UNIT_TYPEID, float> m_army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0} };

		bool m_scv_killer_queued = false;
	};
}