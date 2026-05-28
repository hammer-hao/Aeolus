#include "contingency.h"

#include "forward_pressure.h"

#include "bot_state.h"
#include "base_state.h"
#include "../buildorder/contingency_plan.h"
#include "../Aeolus.h"

#include "../behaviors/macro_behaviors/auto_supply.h"
#include "../behaviors/macro_behaviors/production_controller.h"
#include "../behaviors/macro_behaviors/spawn_controller.h"
#include "../behaviors/macro_behaviors/build_structure.h"
#include "../behaviors/macro_behaviors/tech_up.h"

namespace Aeolus
{
	ContingencyState::ContingencyState(const ContingencyPlan& contingencyPlan) : m_plan(contingencyPlan), m_build_defense_queued(false)
	{
	}

	std::string_view ContingencyState::getName() const
	{
		return "CONTINGENCY";
	}

	void ContingencyState::micro(AeolusBot& aeolusbot)
	{

	}

	void ContingencyState::macro(AeolusBot& aeolusbot)
	{
		doBookKeepingMacroTasks(aeolusbot);

		bool stillBuildingForge = std::make_unique<TechUp>(::sc2::UNIT_TYPEID::PROTOSS_FORGE)->execute(aeolusbot);
		if (!m_build_defense_queued && !stillBuildingForge)
		{
			for (int i = 0; i < m_plan.cannons_to_add; ++i)
			{
				const ::sc2::UNIT_TYPEID to_build = ::sc2::UNIT_TYPEID::PROTOSS_PHOTONCANNON;
				const int base_location = 1;
				const bool is_wall = true;
				aeolusbot.RegisterBehavior(std::make_unique<BuildStructure>(to_build, base_location, is_wall));
			}
			m_build_defense_queued = true;
		}

		// auto supply since we are no longer relying on a build order
		aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		const std::map<::sc2::UNIT_TYPEID, float> armyComp(m_plan.army_composition.begin(), m_plan.army_composition.end());
		aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(armyComp));
		aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(armyComp));

		if (aeolusbot.Observation()->GetFoodUsed() > m_plan.move_out_supply)
		{
			aeolusbot.ChangeState(MakeState<ForwardPressureState>());
		}
	}
}