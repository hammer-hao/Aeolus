#include "bot_state.h"
#include "consolidate.h"

#include "../Aeolus.h"

#include "../behaviors/macro_behaviors/mining.h"
#include "../behaviors/macro_behaviors/build_workers.h"
#include "../behaviors/macro_behaviors/chrono_controller.h"
#include "../behaviors/macro_behaviors/repower_structures.h"
#include "../behaviors/macro_behaviors/auto_supply.h"
#include "../behaviors/macro_behaviors/build_geysers.h"
#include "../behaviors/macro_behaviors/expand.h"
#include "../behaviors/macro_behaviors/production_controller.h"
#include "../behaviors/macro_behaviors/spawn_controller.h"
#include "../behaviors/macro_behaviors/upgrades_controller.h"
#include "../behaviors/macro_behaviors/build_detection.h"
#include "../behaviors/macro_behaviors/scout.h"

#include "../behaviors/micro_behaviors/micro_behavior.h"
#include "../behaviors/micro_behaviors/a_move.h"
#include "../behaviors/micro_behaviors/shoot_target_in_range.h"
#include "../behaviors/micro_behaviors/stutter_unit_back.h"
#include "../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../behaviors/micro_behaviors/path_to_target.h"
#include "../behaviors/micro_behaviors/unload.h"
#include "../behaviors/micro_behaviors/pick_unit_up.h"
#include "../behaviors/micro_behaviors/use_ability.h"

#include "forward_pressure.h"

#include "../utils/unit_utils.h"
#include "../utils/position_utils.h"

#include <iostream>

namespace Aeolus
{
    std::string_view ConsolidateState::getName() const {
        return "CONSOLIDATE";
    }

	void ConsolidateState::macro(AeolusBot& aeolusbot)
	{
        doBookKeepingMacroTasks(aeolusbot);

        doHighEconomyMacroTasks(aeolusbot, true); // force build detection since we want to be as safe as possible

        if (aeolusbot.Observation()->GetGameLoop() % 100 == 0)
        {
            auto ownAttacking = ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
            ::sc2::Units own_army;
            ::sc2::Units opponent_army;
            ::sc2::Units opponent_static_defenses = ManagerMediator::getInstance().GetAllEnemyStaticDefenses(aeolusbot);

            for (const auto* unit : ownAttacking) own_army.push_back(unit);
            auto opponent_units = ManagerMediator::getInstance().GetAllSeenEnemyUnits(aeolusbot);

            for (const auto& unit : opponent_units)
            {
                if (unit->unit_type != ::sc2::UNIT_TYPEID::PROTOSS_PROBE &&
                    unit->unit_type != ::sc2::UNIT_TYPEID::TERRAN_SCV &&
                    unit->unit_type != ::sc2::UNIT_TYPEID::ZERG_DRONE)
                    opponent_army.push_back(unit);
            }

            CombatSimulationResult combatResult = ManagerMediator::getInstance().PredictEngagement(aeolusbot, own_army, opponent_army, opponent_static_defenses);
            float armyRemainingDifference = combatResult.supplyPercentageRemaining - combatResult.enemySupplyPercentageRemaining;
            if (armyRemainingDifference > 0.7f || aeolusbot.Observation()->GetFoodUsed() >= 195)
            {
                aeolusbot.ChangeState(MakeState<ForwardPressureState>());
            }
        }
	}

	void ConsolidateState::micro(AeolusBot& aeolusbot)
	{
        auto& mediator = ManagerMediator::getInstance();

        ::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
        if (!forces.empty()) doGeneralMicro(aeolusbot, forces, mediator.GetDefenseTarget(aeolusbot, 1));

        doPrismPickUpMicro(aeolusbot);

        doOracleHarassMicro(aeolusbot);

        doObserverMicro(aeolusbot);

        // Perform Adept Harassment Micro
        doAdeptHarassMicro(aeolusbot);
	}
}