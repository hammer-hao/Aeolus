#include "base_state.h"

#include "bot_state.h"
#include "../../Aeolus.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>
#include <string>
#include <iostream>

#include "../managers/manager_mediator.h"

#include "../behaviors/macro_behaviors/mining.h"
#include "../behaviors/macro_behaviors/scout.h"
#include "../behaviors/macro_behaviors/build_workers.h"
#include "../behaviors/macro_behaviors/chrono_controller.h"
#include "../behaviors/macro_behaviors/repower_structures.h"
#include "../behaviors/macro_behaviors/build_geysers.h"
#include "../behaviors/macro_behaviors/expand.h"
#include "../behaviors/macro_behaviors/auto_supply.h"
#include "../behaviors/macro_behaviors/build_detection.h"
#include "../behaviors/macro_behaviors/production_controller.h"
#include "../behaviors/macro_behaviors/spawn_controller.h"
#include "../behaviors/macro_behaviors/upgrades_controller.h"

#include "../behaviors/micro_behaviors/micro_behavior.h"
#include "../behaviors/micro_behaviors/a_move.h"
#include "../behaviors/micro_behaviors/shoot_target_in_range.h"
#include "../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../behaviors/micro_behaviors/stutter_unit_back.h"
#include "../behaviors/micro_behaviors/path_to_target.h"
#include "../behaviors/micro_behaviors/pick_unit_up.h"
#include "../behaviors/micro_behaviors/unload.h"
#include "../behaviors/micro_behaviors/use_ability.h"

#include "../utils/unit_utils.h"
#include "../utils/position_utils.h"

namespace Aeolus
{
	void BaseState::declareEnter() 
	{
		std::cout << "[State] Entered state: "
			<< getName() << std::endl;
	}

	void BaseState::declareExit()
	{
		std::cout << "[State] Exited state: "
			<< getName() << std::endl;
	}

	void BaseState::doBookKeepingMacroTasks(AeolusBot& aeolusbot)
	{
		aeolusbot.RegisterBehavior(std::make_unique<Mining>());
		aeolusbot.RegisterBehavior(std::make_unique<Scout>());
		aeolusbot.RegisterBehavior(std::make_unique<BuildWorkers>(
			std::min(ManagerMediator::getInstance().GetOwnReadyTownHalls(aeolusbot).size() * 22, static_cast<size_t>(86))
		));
		aeolusbot.RegisterBehavior(std::make_unique<ChronoController>());
		aeolusbot.RegisterBehavior(std::make_unique<RepowerStructures>());
	}

	void BaseState::doGeneralMicro(AeolusBot& aeolusbot, ::sc2::Units forces, ::sc2::Point2D target)
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

    void BaseState::doPrismPickUpMicro(AeolusBot& aeolusbot)
    {
        auto& mediator = ManagerMediator::getInstance();
        ::sc2::Units ownPrisms = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::PRISM);
        auto prismTarget = mediator.GetPrismTarget(aeolusbot);

        if (prismTarget == ::sc2::Point2D(0.0f, 0.0f)) return;

        for (const auto* prism : ownPrisms)
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
    };

    void BaseState::doOracleHarassMicro(AeolusBot& aeolusbot)
    {
        auto& mediator = ManagerMediator::getInstance();
        std::vector<::sc2::Point2D> harassLocations = aeolusbot.Observation()->GetGameInfo().enemy_start_locations;
        ::sc2::Units oracles = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ORACLE);

        // use single harass location for now
        ::sc2::Point2D harassLocation = harassLocations.front();

        std::vector<::sc2::Point2D> starting_points;
        float search_radius = 12.0f;
        for (const auto& unit : oracles) starting_points.push_back(unit->pos);
        auto enemies_in_range = ManagerMediator::getInstance().GetEnemyUnitsInRangeMap(aeolusbot,
            starting_points, search_radius);

        for (int i = 0; i < oracles.size(); ++i)
        {
            const ::sc2::Unit* oracle = oracles[i];
            auto oracle_behavior = std::make_unique<MicroBehavior>(oracle);
            auto availableAbilities = aeolusbot.Query()->GetAbilitiesForUnit(oracle);

            // 1st priority: keep oracle safe
            if (oracle->shield / oracle->shield_max < 0.3) oracle_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());

            ::sc2::Units workersInRange;
            std::copy_if(enemies_in_range[i].begin(), enemies_in_range[i].end(), std::back_inserter(workersInRange), [](const ::sc2::Unit* unit)
                {
                    return (constants::WORKER_TYPES.find(unit->unit_type) != constants::WORKER_TYPES.end());
                }
            );

            if (!workersInRange.empty() && oracle->energy > 55.0) {
                for (const auto& availableAbility : availableAbilities.abilities)
                {
                    if (availableAbility.ability_id == ::sc2::ABILITY_ID::BEHAVIOR_PULSARBEAMON)
                    {
                        // add activate pulsar beam to behavior
                        oracle_behavior->AddBehavior(std::make_unique<UseAbility>(::sc2::ABILITY_ID::BEHAVIOR_PULSARBEAMON));
                        break;
                    }
                }

                // no activate pulsar beam available means we have already done it.
                auto in_attack_range = ManagerMediator::getInstance().GetUnitsInAtttackRange(aeolusbot, oracle, workersInRange);
                if (!in_attack_range.empty())
                {
                    oracle_behavior->AddBehavior(
                        std::make_unique<ShootTargetInRange>(
                            in_attack_range
                        )
                    );
                }
            }

            // 2nd priority: go to harass target / home for recharge
            ::sc2::Point2D pathTarget = harassLocation;

            for (const auto& availableAbility : availableAbilities.abilities)
            {
                if (availableAbility.ability_id == ::sc2::ABILITY_ID::BEHAVIOR_PULSARBEAMON)
                {
                    // currently no beam activated
                    if (oracle->energy < 50.0f)
                    {
                        pathTarget = utils::GetClosestUnitTo(
                            oracle->pos,
                            ManagerMediator::getInstance().GetOwnReadyTownHalls(aeolusbot)
                        )->pos;
                    }
                    break;
                }
            }
            oracle_behavior->AddBehavior(std::make_unique<PathToTarget>(pathTarget));

            aeolusbot.RegisterBehavior(std::move(oracle_behavior));
        }
    }

    void BaseState::doAdeptHarassMicro(AeolusBot& aeolusbot)
    {
        auto& mediator = ManagerMediator::getInstance();
        ::sc2::Units adepts = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::HARASS_ADEPT);

        std::vector<std::vector<::sc2::Point2D>> positionsBehindEnemyMainNaturalThirdBase = 
            mediator.getPositionsBehindEnemyMainNaturalThird(aeolusbot);

        auto harassmentTracker = mediator.getHarassmentTracker(aeolusbot);

        for (const auto& adept : adepts)
        {
            if (harassmentTracker.find(adept->tag) == harassmentTracker.end())
            {
                mediator.registerHarassmentStatus(aeolusbot, adept->tag, HarassmentStatus::HEADING_TO_BASE);
                continue;
            }
            HarassmentStatus currentStatus = harassmentTracker[adept->tag];

            if (currentStatus == HarassmentStatus::HEADING_TO_BASE)
            {

            }
            else if (currentStatus == HarassmentStatus::HARASSING_AT_MAIN)
            {

            }
            else if (currentStatus == HarassmentStatus::HARASSING_AT_NATURAL)
            {

            }
            else if (currentStatus == HarassmentStatus::SURVIVING)
            {

            }
        }
    }

    void BaseState::doHighEconomyMacroTasks(AeolusBot& aeolusbot, bool forceDetection)
    {
        aeolusbot.RegisterBehavior(std::make_unique<BuildGeysers>());
        aeolusbot.RegisterBehavior(std::make_unique<Expand>());
        aeolusbot.RegisterBehavior(std::make_unique<AutoSupply>());

        aeolusbot.RegisterBehavior(std::make_unique<BuildDetection>(forceDetection));
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
    }

    void BaseState::doObserverMicro(AeolusBot& aeolusbot)
    {
        auto& mediator = ManagerMediator::getInstance();
        ::sc2::Units observers = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::MOBILE_DETECTION);
        for (const auto* observer : observers)
        {
            auto observer_behavior = std::make_unique<MicroBehavior>(observer);
            observer_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
            observer_behavior->AddBehavior(std::make_unique<PathToTarget>(mediator.GetAtttackTarget(aeolusbot)));

            aeolusbot.RegisterBehavior(std::move(observer_behavior));
        }
    }

} // namespace Aeolus