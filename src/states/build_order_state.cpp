#include "build_order_state.h"

#include "../behaviors/macro_behaviors/mining.h"
#include "../behaviors/macro_behaviors/build_workers.h"
#include "../behaviors/macro_behaviors/chrono_controller.h"
#include "../behaviors/macro_behaviors/repower_structures.h"
#include "../behaviors/macro_behaviors/auto_supply.h"

#include "forward_pressure.h"

#include "../behaviors/micro_behaviors/micro_behavior.h"
#include "../behaviors/micro_behaviors/a_move.h"
#include "../behaviors/micro_behaviors/shoot_target_in_range.h"
#include "../behaviors/micro_behaviors/stutter_unit_back.h"
#include "../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../behaviors/micro_behaviors/path_to_target.h"
#include "../behaviors/micro_behaviors/unload.h"
#include "../behaviors/micro_behaviors/pick_unit_up.h"

#include "../utils/unit_utils.h"

#include "../Aeolus.h"

namespace Aeolus
{
	void BuildOrderState::OnEnter(AeolusBot& aeolusbot)
	{
		std::cout << "[State] Entered state: BUILDORDER" << std::endl;
	}

	void BuildOrderState::OnExit()
	{
		std::cout << "[State] Exited state: BUILDORDER" << std::endl;
	}

	void BuildOrderState::macro(AeolusBot& aeolusbot)
	{
		// execute the build first
		aeolusbot.ExecuteBuildOrder();

		// things to keep track of while the build order is running
		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(
			std::min(ManagerMediator::getInstance().GetOwnReadyTownHalls(aeolusbot).size() * 22, static_cast<size_t>(86))
		));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

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

		if (!forces.empty()) _micro(aeolusbot, forces, target);

		::sc2::Units prisms = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::PRISM);
		auto prismTarget = mediator.GetPrismTarget(aeolusbot);
		if (prismTarget != ::sc2::Point2D(0.0f, 0.0f)) _prismMicro(aeolusbot, prisms, prismTarget);
	}

    void BuildOrderState::_micro(AeolusBot& aeolusbot, ::sc2::Units forces, ::sc2::Point2D target)
    {
        std::vector<::sc2::Point2D> starting_points;
        float search_radius = 15.0f;
        for (const auto& unit : forces) starting_points.push_back(unit->pos);
        auto enemies_in_range = ManagerMediator::getInstance().GetEnemyUnitsInRangeMap(aeolusbot,
            starting_points, search_radius);

        for (int i = 0; i < forces.size(); ++i)
        {
            const ::sc2::Unit* unit = forces[i];

            // 1) Create the MicroBehavior as a unique_ptr
            auto combat_behavior = std::make_unique<MicroBehavior>(unit);

            // 2) Filter out close enemies
            ::sc2::Units close_units;
            for (const auto& enemy : enemies_in_range[i])
                if (enemy->display_type != ::sc2::Unit::DisplayType::Snapshot
                    && constants::IGNORED_UNITS.find(enemy->unit_type) == constants::IGNORED_UNITS.end())
                    close_units.push_back(enemy);

            ::sc2::Units close_non_structures;
            for (const auto& enemy : close_units) if (constants::ALL_STRUCTURES.find(enemy->unit_type) == constants::ALL_STRUCTURES.end())
                close_non_structures.push_back(enemy);

            // Add the path behavior if no close enemy is spotted
            if (!close_units.empty())
            {
                auto in_attack_range = ManagerMediator::getInstance().GetUnitsInAtttackRange(aeolusbot, unit, close_non_structures);
                if (!in_attack_range.empty())
                {
                    combat_behavior->AddBehavior(
                        std::make_unique<ShootTargetInRange>(
                            in_attack_range
                        )
                    );
                }
                else
                {
                    auto all_in_attack_range = ManagerMediator::getInstance().GetUnitsInAtttackRange(aeolusbot, unit, close_units);
                    if (!all_in_attack_range.empty())
                    {
                        combat_behavior->AddBehavior(
                            std::make_unique<ShootTargetInRange>(
                                all_in_attack_range
                            )
                        );
                    }
                }

                auto enemy_target = utils::PickAttackTarget(close_units);

                if ((unit->shield / unit->shield_max) < 0.1)
                {
                    combat_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
                }
                else
                {
                    combat_behavior->AddBehavior(std::make_unique<StutterUnitBack>(enemy_target));
                }
            }
            else
            {
                combat_behavior->AddBehavior(
                    std::make_unique<PathToTarget>(
                        target
                    ));

                combat_behavior->AddBehavior(
                    std::make_unique<AMove>(
                        target
                    ));
            }

            // Now register the combat behavior
            aeolusbot.RegisterBehavior(std::move(combat_behavior));
        }
    }

    void BuildOrderState::_prismMicro(AeolusBot& aeolusbot, ::sc2::Units prisms, ::sc2::Point2D prismTarget)
    {
        for (const auto* prism : prisms)
        {
            auto combat_behavior = std::make_unique<MicroBehavior>(prism);

            // 1st priority: keep prism safe
            combat_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());

            // 2nd priority: unload all units inside
            combat_behavior->AddBehavior(std::make_unique<Unload>());

            // 3rd/main priority: prick up endangered units
            ::sc2::Units inPickupRange = ManagerMediator::getInstance().GetOwnAttackingUnitsInRange(aeolusbot, { prism->pos }, 5.0f);
            if (!inPickupRange.empty())
            {
                for (const auto* unit : inPickupRange)
                {
                    if ((unit->shield / unit->shield_max) <= 0.01f && !ManagerMediator::getInstance().IsGroundPositionSafe(aeolusbot, unit->pos))
                    {
                        combat_behavior->AddBehavior(std::make_unique<PickUnitUp>(unit));
                        break;
                    }
                }
            }

            bool updatePath = true;
            if (prism->orders.size() == 1)
            {
                if (prism->orders.front().ability_id == ::sc2::ABILITY_ID::GENERAL_MOVE
                    && ::sc2::DistanceSquared2D(prism->orders.front().target_pos, prismTarget) < 4.0f)
                    updatePath = false;
            }

            // nothing else to do: path the prism to our optimal position
            if (updatePath) combat_behavior->AddBehavior(std::make_unique<PathToTarget>(prismTarget));

            aeolusbot.RegisterBehavior(std::move(combat_behavior));
        }
    }
}