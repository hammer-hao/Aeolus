#include "base_state.h"

#include "bot_state.h"
#include "../../Aeolus.h"
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_common.h>
#include <sc2api/sc2_data.h>
#include <string>
#include <iostream>

#include "../managers/manager_mediator.h"
#include "../Aeolus.h"
#include "../enums.h"

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
#include "../behaviors/micro_behaviors/move.h"
#include "../behaviors/micro_behaviors/move_toward_target_safely.h"

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

        ::sc2::Units oracles =
            mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::ORACLE);

        auto unitsInRangeMap = _getEnemiesInRangeMap(aeolusbot, oracles);

        auto harassmentTracker =
            mediator.getHarassmentTracker(aeolusbot);

        std::vector<std::vector<::sc2::Point2D>>
            positionsBehindEnemyMainNaturalThirdBase =
            mediator.getPositionsBehindEnemyMainNaturalThird(aeolusbot);

        for (const auto* oracle : oracles)
        {
            if (harassmentTracker.find(oracle->tag) ==
                harassmentTracker.end())
            {
                mediator.registerHarassmentStatus(
                    aeolusbot,
                    oracle->tag,
                    HarassmentStatus::HEADING_TO_BASE);

                continue;
            }

            auto oracle_behavior =
                std::make_unique<MicroBehavior>(oracle);

            auto availableAbilities =
                aeolusbot.Query()->GetAbilitiesForUnit(oracle).abilities;

            auto beamAvailable =
                std::any_of(
                    availableAbilities.begin(),
                    availableAbilities.end(),
                    [](const ::sc2::AvailableAbility& ability)
                    {
                        return ability.ability_id ==
                            ::sc2::ABILITY_ID::BEHAVIOR_PULSARBEAMON;
                    });

            bool beamCurrentlyOff = beamAvailable;
            HarassmentStatus currentStatus =
                harassmentTracker[oracle->tag];

            const auto& allClose =
                unitsInRangeMap[oracle->tag];

            ::sc2::Units closeWorkers;

            std::copy_if(
                allClose.begin(),
                allClose.end(),
                std::back_inserter(closeWorkers),
                [](const ::sc2::Unit* unit)
                {
                    return constants::WORKER_TYPES.find(
                        unit->unit_type)
                        != constants::WORKER_TYPES.end();
                });

            // ----------------------------
            // HEADING TO BASE
            // ----------------------------

            if (currentStatus ==
                HarassmentStatus::HEADING_TO_BASE)
            {
                ::sc2::Point2D target =
                    positionsBehindEnemyMainNaturalThirdBase
                    .front()[1];

                oracle_behavior->AddBehavior(
                    std::make_unique<AMove>(target));

                if (closeWorkers.size() >= 3)
                {
                    HarassmentStatus newStatus =
                        _getClosestHarassStatus(
                            aeolusbot,
                            oracle,
                            positionsBehindEnemyMainNaturalThirdBase);

                    mediator.registerHarassmentStatus(
                        aeolusbot,
                        oracle->tag,
                        newStatus);
                }
            }

            // ----------------------------
            // HARASSING
            // ----------------------------

            else if (
                currentStatus ==
                HarassmentStatus::HARASSING_AT_MAIN ||
                currentStatus ==
                HarassmentStatus::HARASSING_AT_NATURAL ||
                currentStatus ==
                HarassmentStatus::HARASSING_AT_THIRD)
            {
                if ((oracle->shield / oracle->shield_max)
                    <= 0.15f)
                {
                    mediator.registerHarassmentStatus(
                        aeolusbot,
                        oracle->tag,
                        HarassmentStatus::SURVIVING);
                }

                if (!closeWorkers.empty())
                {
                    if (beamCurrentlyOff &&
                        oracle->energy > 40.0f)
                    {
                        oracle_behavior->AddBehavior(
                            std::make_unique<UseAbility>(
                                ::sc2::ABILITY_ID::
                                BEHAVIOR_PULSARBEAMON));
                    }

                    auto workersInAttackRange =
                        mediator.GetUnitsInAtttackRange(
                            aeolusbot,
                            oracle,
                            closeWorkers);

                    if (!workersInAttackRange.empty())
                    {
                        oracle_behavior->AddBehavior(
                            std::make_unique<
                            ShootTargetInRange>(
                                workersInAttackRange));

                        oracle_behavior->AddBehavior(
                            std::make_unique<
                            KeepUnitSafe>());
                    }
                    else
                    {
                        auto enemyTarget =
                            utils::PickAttackTarget(
                                closeWorkers);

                        oracle_behavior->AddBehavior(
                            std::make_unique<
                            StutterUnitBack>(
                                enemyTarget));
                    }
                }
                else
                {
                    // no workers nearby
                    oracle_behavior->AddBehavior(
                        std::make_unique<KeepUnitSafe>());
                }
            }

            // ----------------------------
            // SURVIVING
            // ----------------------------

            else if (currentStatus ==
                HarassmentStatus::SURVIVING)
            {
                if (!beamCurrentlyOff)
                {
                    oracle_behavior->AddBehavior(
                        std::make_unique<UseAbility>(
                            ::sc2::ABILITY_ID::
                            BEHAVIOR_PULSARBEAMOFF));
                }

                oracle_behavior->AddBehavior(
                    std::make_unique<KeepUnitSafe>());

                oracle_behavior->AddBehavior(
                    std::make_unique<PathToTarget>(
                        aeolusbot.Observation()
                        ->GetStartLocation()));

                if ((oracle->shield /
                    oracle->shield_max) >= 0.95f
                    && oracle->energy >= 80.0f)
                {
                    mediator.registerHarassmentStatus(
                        aeolusbot,
                        oracle->tag,
                        HarassmentStatus::HEADING_TO_BASE);
                }
            }

            aeolusbot.RegisterBehavior(
                std::move(oracle_behavior));
        }
    }

    void BaseState::doAdeptHarassMicro(AeolusBot& aeolusbot)
    {
        auto& mediator = ManagerMediator::getInstance();
        ::sc2::Units adepts = mediator.GetUnitsFromRole(aeolusbot, constants::UnitRole::HARASS_ADEPT);
        std::map<::sc2::Tag, ::sc2::Units> unitsInRangeMap = _getEnemiesInRangeMap(aeolusbot, adepts);

        std::vector<std::vector<::sc2::Point2D>> positionsBehindEnemyMainNaturalThirdBase =
            mediator.getPositionsBehindEnemyMainNaturalThird(aeolusbot);

        auto harassmentTracker = mediator.getHarassmentTracker(aeolusbot);
        auto adeptShadeTracker = mediator.GetAdeptShadeTracker(aeolusbot);

        for (const auto& adept : adepts)
        {
            if (harassmentTracker.find(adept->tag) == harassmentTracker.end())
            {
                mediator.registerHarassmentStatus(aeolusbot, adept->tag, HarassmentStatus::HEADING_TO_BASE);
                continue;
            }

            auto adept_behavior = std::make_unique<MicroBehavior>(adept);
            auto available_abilities = aeolusbot.Query()->GetAbilitiesForUnit(adept).abilities;
            if (std::any_of(available_abilities.begin(), available_abilities.end(), [](::sc2::AvailableAbility ability) {
                return ability.ability_id == ::sc2::ABILITY_ID::EFFECT_ADEPTPHASESHIFT;
                })) {
                adept_behavior->AddBehavior(std::make_unique<UseAbility>(::sc2::ABILITY_ID::EFFECT_ADEPTPHASESHIFT, adept->pos));
            }

            HarassmentStatus currentStatus = harassmentTracker[adept->tag];

            if (currentStatus == HarassmentStatus::HEADING_TO_BASE)
            {
                std::vector<::sc2::Point2D> pointsBehindMain =
                    positionsBehindEnemyMainNaturalThirdBase.front();

                /*::sc2::Point2D closest = *std::min_element(
                    pointsBehindMain.begin(), pointsBehindMain.end(),
                    [adept, &aeolusbot](::sc2::Point2D a, ::sc2::Point2D b) {
                        return aeolusbot.Query()->PathingDistance(adept->pos, a) < aeolusbot.Query()->PathingDistance(adept->pos, b);
                    }
                );*/

                ::sc2::Point2D closest = pointsBehindMain[1];

                if (!unitsInRangeMap[adept->tag].empty())
                    adept_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
                adept_behavior->AddBehavior(std::make_unique<AMove>(closest));

                if (std::count_if(unitsInRangeMap[adept->tag].begin(), unitsInRangeMap[adept->tag].end(), [](const ::sc2::Unit* unit) {
                    return (constants::WORKER_TYPES.find(unit->unit_type) != constants::WORKER_TYPES.end());
                    }) >= 3)
                {
                    HarassmentStatus newStatus = _getClosestHarassStatus(aeolusbot, adept, positionsBehindEnemyMainNaturalThirdBase);
                    mediator.registerHarassmentStatus(aeolusbot, adept->tag, newStatus);
                }
            }
            else if (currentStatus == HarassmentStatus::HARASSING_AT_MAIN || currentStatus == HarassmentStatus::HARASSING_AT_NATURAL)
            {
                const ::sc2::Units& allClose = unitsInRangeMap[adept->tag];

                if (!std::any_of(allClose.begin(), allClose.end(), [](const ::sc2::Unit* unit) {
                    return (constants::WORKER_TYPES.find(unit->unit_type) != constants::WORKER_TYPES.end());
                    })) {
                    adept_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
                }
                else 
                {
                    ::sc2::Units closeWorkers;
                    std::copy_if(allClose.begin(), allClose.end(), std::back_inserter(closeWorkers), [](const ::sc2::Unit* unit) {
                        return (constants::WORKER_TYPES.find(unit->unit_type) != constants::WORKER_TYPES.end());
                        });
                    auto workserInAttackRange = ManagerMediator::getInstance().GetUnitsInAtttackRange(aeolusbot, adept, closeWorkers);
                    if (!workserInAttackRange.empty())
                    {
                        adept_behavior->AddBehavior(
                            std::make_unique<ShootTargetInRange>(
                                workserInAttackRange
                            )
                        );
                        adept_behavior->AddBehavior(
                            std::make_unique<MoveTowardTargetSafely>(
                                workserInAttackRange
                            )
                        );
                        adept_behavior->AddBehavior(
                            std::make_unique<KeepUnitSafe>()
                        );
                    }
                    else
                    {
                        auto enemy_target = utils::PickAttackTarget(allClose);
                        adept_behavior->AddBehavior(std::make_unique<StutterUnitBack>(enemy_target));
                    }
                }

                if ((adept->shield / adept->shield_max) <= 0.05f)
                {
                    mediator.registerHarassmentStatus(aeolusbot, adept->tag, HarassmentStatus::SURVIVING);
                }
            }
            else if (currentStatus == HarassmentStatus::SURVIVING)
            {
                const ::sc2::Units& allClose = unitsInRangeMap[adept->tag];
                ::sc2::Units closeWorkers;
                std::copy_if(allClose.begin(), allClose.end(), std::back_inserter(closeWorkers), [](const ::sc2::Unit* unit) {
                    return (constants::WORKER_TYPES.find(unit->unit_type) != constants::WORKER_TYPES.end());
                    });
                auto workserInAttackRange = ManagerMediator::getInstance().GetUnitsInAtttackRange(aeolusbot, adept, closeWorkers);
                if (!workserInAttackRange.empty())
                {
                    adept_behavior->AddBehavior(
                        std::make_unique<ShootTargetInRange>(
                            workserInAttackRange
                        )
                    );
                }
                adept_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
                if ((adept->shield / adept->shield_max) >= 0.95f)
                {
                    mediator.registerHarassmentStatus(aeolusbot, adept->tag, HarassmentStatus::HEADING_TO_BASE);
                }
            }

            aeolusbot.RegisterBehavior(std::move(adept_behavior));

            if (adeptShadeTracker.find(adept->tag) != adeptShadeTracker.end())
            {
                const ::sc2::Unit* adeptShade = aeolusbot.Observation()->GetUnit(adeptShadeTracker[adept->tag].first);
                auto adept_shade_behavior = std::make_unique<MicroBehavior>(adeptShade);

                if (adeptShadeTracker[adept->tag].second <= 44)
                {
                    adept_shade_behavior->AddBehavior(std::make_unique<KeepUnitSafe>());
                }

                auto target = _getAdeptShadeTargetFromHarassmentStatus(currentStatus,
                    positionsBehindEnemyMainNaturalThirdBase,
                    aeolusbot.Observation()->GetStartLocation());
                adept_shade_behavior->AddBehavior(std::make_unique<Move>(target));

                if (adeptShadeTracker[adept->tag].second == 1)
                {
                    if (currentStatus == HarassmentStatus::SURVIVING || currentStatus == HarassmentStatus::HEADING_TO_BASE)
                    {

                    }
                    else
                    {
                        // mediator.IsGroundPositionSafe()
                        // need to decide if cancel or not
                        // adept_shade_behavior->AddBehavior(std::make_unique<UseAbility>(::sc2::ABILITY_ID::CANCEL));
                        HarassmentStatus newStatus = _getClosestHarassStatus(aeolusbot, adeptShade, positionsBehindEnemyMainNaturalThirdBase);
                        mediator.registerHarassmentStatus(aeolusbot, adept->tag, newStatus);
                    }
                }

                aeolusbot.RegisterBehavior(std::move(adept_shade_behavior));
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

    std::map<::sc2::Tag, ::sc2::Units> BaseState::_getEnemiesInRangeMap(AeolusBot& aeolusbot, const ::sc2::Units& units)
    {
        std::map<::sc2::Tag, ::sc2::Units> out;
        auto& mediator = ManagerMediator::getInstance();

        std::vector<::sc2::Point2D> startingPoints(units.size());

        std::transform(units.begin(), units.end(), startingPoints.begin(), [](const ::sc2::Unit* unit) {
            return unit->pos;
            });

        const auto queryResult = mediator.GetEnemyUnitsInRangeMap(aeolusbot, startingPoints, 5.0);

        for (int i = 0; i < units.size(); ++i)
        {
            out.insert({units[i]->tag, queryResult[i]});
        }
        return out;
    }

    ::sc2::Point2D BaseState::_getAdeptShadeTargetFromHarassmentStatus(
        HarassmentStatus status, 
        const std::vector<std::vector<::sc2::Point2D>>& behindMineralPositions, ::sc2::Point2D fallbackLocation)
    {
        if (status == HarassmentStatus::HEADING_TO_BASE)
        {
            return behindMineralPositions.front()[1];
        }
        else if (status == HarassmentStatus::HARASSING_AT_MAIN)
        {
            return behindMineralPositions[1][1];
        }
        else if (status == HarassmentStatus::HARASSING_AT_NATURAL)
        {
            return behindMineralPositions.front()[1];
        }
        else if (status == HarassmentStatus::HARASSING_AT_THIRD)
        {
            return behindMineralPositions[1][1];
        }
        else if (status == HarassmentStatus::SURVIVING)
        {
            return fallbackLocation;
        }
        return behindMineralPositions.front().front();
    }

    HarassmentStatus BaseState::_getClosestHarassStatus(
        AeolusBot& aeolusbot,
        const ::sc2::Unit* adept,
        const std::vector<std::vector<::sc2::Point2D>>& positionsBehindEnemyMainNaturalThirdBase)
    {
        auto* query = aeolusbot.Query();

        auto distanceToGroup = [&](const std::vector<::sc2::Point2D>& points) {
            if (points.empty()) {
                return std::numeric_limits<float>::max();
            }

            auto closestPointIt = std::min_element(
                points.begin(),
                points.end(),
                [&](const ::sc2::Point2D& a, const ::sc2::Point2D& b) {
                    return query->PathingDistance(adept->pos, a)
                        < query->PathingDistance(adept->pos, b);
                });

            return query->PathingDistance(adept->pos, *closestPointIt);
            };

        size_t bestIndex = 0;
        float bestDistance = std::numeric_limits<float>::max();

        for (size_t i = 0; i < positionsBehindEnemyMainNaturalThirdBase.size(); ++i)
        {
            float dist = distanceToGroup(positionsBehindEnemyMainNaturalThirdBase[i]);

            if (dist < bestDistance && dist!=0)
            {
                bestDistance = dist;
                bestIndex = i;
            }
        }

        switch (bestIndex)
        {
        case 0:
            return HarassmentStatus::HARASSING_AT_MAIN;
        case 1:
            return HarassmentStatus::HARASSING_AT_NATURAL;
        case 2:
            return HarassmentStatus::HARASSING_AT_THIRD;
        default:
            return HarassmentStatus::HARASSING_AT_MAIN;
        }
    }
} // namespace Aeolus