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
	* @brief The proxy pylon state is entered when we detect an enemy
	* pylon proxied on the map during the build order stage. Note this can
	* be triggered by partially detecting a proxy gateway or cannon rush.
	*/
	class ProxyPylon : public ContingencyState
	{
	public:
		ProxyPylon() {};

		std::string_view getName() const override;

		void micro(AeolusBot& aeolusbot) override;

		void macro(AeolusBot& aeolusbot) override;

		void OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit* unit) override;

	private:
		const ::sc2::Unit* m_scout = nullptr;
		bool m_scout_sent = false;
	};
}