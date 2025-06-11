#include "forward_pressure.h"
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

#include "../behaviors/micro_behaviors/micro_behavior.h"
#include "../behaviors/micro_behaviors/a_move.h"
#include "../behaviors/micro_behaviors/shoot_target_in_range.h"
#include "../behaviors/micro_behaviors/stutter_unit_back.h"
#include "../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../behaviors/micro_behaviors/path_to_target.h"
#include "../behaviors/micro_behaviors/unload.h"
#include "../behaviors/micro_behaviors/pick_unit_up.h"

#include "consolidate.h"

#include "../utils/unit_utils.h"

#include <iostream>

namespace Aeolus
{
	void ForwardPressureState::OnEnter(AeolusBot& aeolusbot)
	{
		std::cout << "entered FOWARD PRESSURE state at gameloop " << aeolusbot.Observation()->GetGameLoop() << std::endl;
        m_enteredAt = aeolusbot.Observation()->GetGameLoop();
	}

	void ForwardPressureState::OnExit()
	{
		std::cout << "exited FORWARD PRESSURE state" << std::endl;
	}

	void ForwardPressureState::macro(AeolusBot& aeolusbot)
	{
        // std::cout << "Aeolus: Macroing..." << std::endl;
        // Implement custom logic for gathering resources, expanding, etc.
        aeolusbot.RegisterBehavior(std::make_unique<Mining>());
        aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(
            std::min(ManagerMediator::getInstance().GetOwnReadyTownHalls(aeolusbot).size() * 22, static_cast<size_t>(86))
        ));
        aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
        aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());

        aeolusbot.RegisterBehavior(std::make_unique<BuildGeysers>());
        aeolusbot.RegisterBehavior(std::make_unique<Expand>());
        aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());
        aeolusbot.RegisterBehavior(std::make_unique<ProductionController>(aeolusbot.getArmyComp()));
        aeolusbot.RegisterBehavior(std::make_unique<SpawnController>(aeolusbot.getArmyComp()));
        aeolusbot.RegisterBehavior(std::make_unique<UpgradesController>(
            std::vector<::sc2::UPGRADE_ID>{
            ::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1,
                ::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2,
                ::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL1,
                ::sc2::UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3,
                ::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL2,
                ::sc2::UPGRADE_ID::PROTOSSSHIELDSLEVEL3,
                ::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1,
                ::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2,
                ::sc2::UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3
        }
        ));

        if (aeolusbot.Observation()->GetGameLoop() % 50 == 0)
            std::cout << "current gameloop: " << aeolusbot.Observation()->GetGameLoop() << std::endl;

        /*
        if (ManagerMediator::getInstance().GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING).size()
            < aeolusbot.getMoveOutSupply())
            aeolusbot.ChangeState(MakeState<ConsolidateState>());
        */
	}

	void ForwardPressureState::micro(AeolusBot& aeolusbot)
	{
        auto& mediator = ManagerMediator::getInstance();

        ::sc2::Units forces = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ATTACKING);
        if (!forces.empty()) _micro(aeolusbot, forces, mediator.GetAtttackTarget(aeolusbot));

        ::sc2::Units prisms = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::PRISM);
        auto prismTarget = mediator.GetPrismTarget(aeolusbot);
        if (prismTarget != ::sc2::Point2D(0.0f, 0.0f)) _prismMicro(aeolusbot, prisms, prismTarget);
	}

    void ForwardPressureState::_micro(AeolusBot& aeolusbot, ::sc2::Units forces, ::sc2::Point2D target)
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

    void ForwardPressureState::_prismMicro(AeolusBot& aeolusbot, ::sc2::Units prisms, ::sc2::Point2D prismTarget)
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

    void ForwardPressureState::OnUnitDestroyed(AeolusBot& aeolusbot, const ::sc2::Unit* unit)
    {
        const uint64_t window = 672; // 30 seconds window
        uint64_t currentLoop = aeolusbot.Observation()->GetGameLoop();

        auto cost = ManagerMediator::getInstance().GetUnitCost(aeolusbot, unit->unit_type);
        if (unit->alliance == ::sc2::Unit::Alliance::Self)
        {
            m_ownLosses.push_back({ currentLoop, cost.first + cost.second });
        }
        if (unit->alliance == ::sc2::Unit::Alliance::Enemy)
        {
            m_opponentLosses.push_back({ currentLoop, cost.first + cost.second });
        }

        while (!m_ownLosses.empty() && currentLoop - m_ownLosses.front().first > window)
        {
            m_ownLosses.pop_front();
        }
        while (!m_opponentLosses.empty() && currentLoop - m_opponentLosses.front().first > window)
        {
            m_opponentLosses.pop_front();
        }

        // in this state for more than 60 seconds, check for loss ratio
        if (currentLoop - m_enteredAt > 1344)
        {
            int ownTotalLoss = 0, opponentTotalLoss = 0;
            for (const auto& loss : m_ownLosses) ownTotalLoss += loss.second;
            for (const auto& loss : m_opponentLosses) opponentTotalLoss += loss.second;

            std::cout << "own loss in the last 30 seconds: " << ownTotalLoss << std::endl;
            std::cout << "opponent loss in the last 30 seconds: " << opponentTotalLoss << std::endl;
            if (static_cast<double>(ownTotalLoss) / static_cast<double>(opponentTotalLoss) > 2.0
                && static_cast<double>(ownTotalLoss) > 1000)
            {
                std::cout << "[State] Forward pressure: trading badly, consolidate and tech switching..." << std::endl;
                aeolusbot.ChangeState(MakeState<ConsolidateState>(static_cast<int>(ownTotalLoss * 1.5)));
            }
            /*
            std::cout << "known enemy units: " << std::endl;
            for (const auto& unit_type : ManagerMediator::getInstance().GetAllSeenEnemyUnits(aeolusbot))
            {
                std::cout << ::sc2::UnitTypeToName(unit_type) << '\n';
            }
            */
        }
    }
}