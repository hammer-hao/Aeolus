#include "forward_pressure.h"
#include "../Aeolus.h"

#include "../behaviors/macro_behaviors/mining.h"
#include "../behaviors/macro_behaviors/build_workers.h"
#include "../behaviors/macro_behaviors/chrono_controller.h"
#include "../behaviors/macro_behaviors/repower_structures.h"
#include "../behaviors/macro_behaviors/auto_supply.h"
#include "../behaviors/macro_behaviors/build_geysers.h"
#include "../behaviors/macro_behaviors/expand.h"
#include "../behaviors/macro_behaviors/tech_up.h"
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

#include "consolidate.h"

#include "../utils/unit_utils.h"
#include "../utils/position_utils.h"

#include <iostream>

namespace Aeolus
{
    std::string_view ForwardPressureState::getName() const {
        return "FORWARD_PRESSURE";
    }

	void ForwardPressureState::macro(AeolusBot& aeolusbot)
	{
        // Bookkeeping tasks first
        doBookKeepingMacroTasks(aeolusbot);

        // research nice-to-have upgrades
        if (aeolusbot.Observation()->GetFoodArmy() > 40) {
            aeolusbot.RegisterBehavior(std::make_unique<TechUp>(::sc2::UPGRADE_ID::BLINKTECH));
        }

        // high economy macro tasks
        doHighEconomyMacroTasks(aeolusbot, false); // forceDetection = false, don't force detection when we don't need it

        if (aeolusbot.Observation()->GetGameLoop() % 100 == 50)
        {
            _transitionIntoConsolidateIfNeeded(aeolusbot);
        }
	}

	void ForwardPressureState::micro(AeolusBot& aeolusbot)
	{
        auto& mediator = ManagerMediator::getInstance();

        ::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
        if (!forces.empty()) doGeneralMicro(aeolusbot, forces, mediator.GetAtttackTarget(aeolusbot));

        doPrismPickUpMicro(aeolusbot);

        doOracleHarassMicro(aeolusbot);

        doObserverMicro(aeolusbot);

        // Perform Adept Harassment Micro
        doAdeptHarassMicro(aeolusbot);
	}

    void ForwardPressureState::_transitionIntoConsolidateIfNeeded(AeolusBot& aeolusbot)
    {
        auto ownAttacking = ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
        std::vector<::sc2::UNIT_TYPEID> own_army;
        std::vector<::sc2::UNIT_TYPEID> opponent_army;

        for (const auto* unit : ownAttacking) own_army.push_back(unit->unit_type);
        auto opponent_units = ManagerMediator::getInstance().GetAllSeenEnemyUnits(aeolusbot);

        for (const auto& unit_type : opponent_units)
        {
            if (unit_type != ::sc2::UNIT_TYPEID::PROTOSS_PROBE &&
                unit_type != ::sc2::UNIT_TYPEID::TERRAN_SCV &&
                unit_type != ::sc2::UNIT_TYPEID::ZERG_DRONE)
                opponent_army.push_back(unit_type);
        }

        bool won_engagement = ManagerMediator::getInstance().PredictEngagement(aeolusbot, own_army, opponent_army);
        if (!won_engagement)
        {
            aeolusbot.ChangeState(MakeState<ConsolidateState>());
            std::cout << "Consolidating: Calculating best army composition..." << std::endl;
        }
    }

    void ForwardPressureState::OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit*)
    {
        return;
    }
}