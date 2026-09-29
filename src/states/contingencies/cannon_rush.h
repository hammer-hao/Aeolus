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
	* @brief The cannon rush response state. We enter this state when
	* we see a photon cannon present near our base
	*/
	class CannonRush : public ContingencyState
	{
	public:
		CannonRush() {}

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*) override;

	private:
		const std::map<::sc2::UNIT_TYPEID, float> m_army_comp = { {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0} };
		std::map<::sc2::Tag, std::unordered_set<const ::sc2::Unit*>> m_pulled_probes;

		void pullProbes(AeolusBot& aeolusbot);

		void unpullAllProbes(AeolusBot& aeolusbot);

		void unpullProbes(AeolusBot& aeolusbot, ::sc2::Tag target);
	};
}