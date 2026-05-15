#include "build_order_state.h"
#include "base_state.h"

#include "../behaviors/macro_behaviors/mining.h"
#include "../behaviors/macro_behaviors/build_workers.h"
#include "../behaviors/macro_behaviors/chrono_controller.h"
#include "../behaviors/macro_behaviors/repower_structures.h"
#include "../behaviors/macro_behaviors/auto_supply.h"
#include "../behaviors/macro_behaviors/scout.h"

#include "forward_pressure.h"

#include "../behaviors/micro_behaviors/micro_behavior.h"
#include "../behaviors/micro_behaviors/a_move.h"
#include "../behaviors/micro_behaviors/shoot_target_in_range.h"
#include "../behaviors/micro_behaviors/stutter_unit_back.h"
#include "../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../behaviors/micro_behaviors/path_to_target.h"
#include "../behaviors/micro_behaviors/unload.h"
#include "../behaviors/micro_behaviors/pick_unit_up.h"
#include "../behaviors/micro_behaviors/use_ability.h"

#include "../utils/unit_utils.h"
#include "../utils/position_utils.h"

#include "../Aeolus.h"

namespace Aeolus
{
    std::string_view BuildOrderState::getName() const {
        return "BUILDORDERSTATE";
    }

	void BuildOrderState::macro(AeolusBot& aeolusbot)
	{
		// execute the build first
		aeolusbot.ExecuteBuildOrder();

        // bookkeeping macro tasks
        doBookKeepingMacroTasks(aeolusbot);

		// auto supply only when we are somehow supply blocked during executing the build order
		if (aeolusbot.Observation()->GetFoodCap() <= aeolusbot.Observation()->GetFoodUsed())
			aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

		// done with the build, default to forward pressuring.
		if (aeolusbot.isBuildFinished()) aeolusbot.ChangeState(MakeState<ForwardPressureState>()); 
	}

	void BuildOrderState::micro(AeolusBot& aeolusbot)
	{
        if (aeolusbot.Observation()->GetGameLoop() < 100) return;
		auto& mediator = ManagerMediator::getInstance();

		// during the build order, we generally want to defend. However, we would still like to move out
		// if we have enought supply
		::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
        
        if (forces.empty()) return;

		::sc2::Point2D target = (forces.size() >= aeolusbot.getMoveOutSupply()) ?
			mediator.GetAtttackTarget(aeolusbot) : mediator.GetDefenseTarget(aeolusbot, 1);

		if (!forces.empty()) doGeneralMicro(aeolusbot, forces, target);

        // Enable prism pick up during build order stage if one has been created
        doPrismPickUpMicro(aeolusbot);

		// We are open to doing Oracle harass during the build order stage
		doOracleHarassMicro(aeolusbot);
	}
}