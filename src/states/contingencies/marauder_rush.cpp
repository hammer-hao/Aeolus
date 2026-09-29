#include "marauder_rush.h"

#include "../contingency.h"
#include "../base_state.h"
#include "../bot_state.h"
#include "../forward_pressure.h"

#include "sc2api/sc2_unit.h"
#include "sc2api/sc2_common.h"

#include "../../managers/manager_mediator.h"
#include "../../Aeolus.h"

#include "../../behaviors/macro_behaviors/mining.h"
#include "../../behaviors/macro_behaviors/scout.h"
#include "../../behaviors/macro_behaviors/production_controller.h"
#include "../../behaviors/macro_behaviors/auto_supply.h"
#include "../../behaviors/macro_behaviors/build_workers.h"
#include "../../behaviors/macro_behaviors/spawn_controller.h"
#include "../../behaviors/macro_behaviors/chrono_controller.h"
#include "../../behaviors/macro_behaviors/repower_structures.h"
#include "../../behaviors/macro_behaviors/build_structure.h"

#include "../../behaviors/micro_behaviors/micro_behavior.h"
#include "../../behaviors/micro_behaviors/use_ability.h"
#include "../../behaviors/micro_behaviors/a_move.h"
#include "../../behaviors/micro_behaviors/path_to_target.h"
#include "../../behaviors/micro_behaviors/shoot_target_in_range.h"
#include "../../behaviors/micro_behaviors/keep_unit_safe.h"
#include "../../behaviors/micro_behaviors/stutter_unit_back.h"

#include "../../utils/unit_utils.h"

#include <algorithm>
#include <map>
#include <memory>

namespace Aeolus
{
    std::string_view MarauderRush::getName() const
    {
        return "MARAUDER_RUSH";
    }

    void MarauderRush::micro(AeolusBot& aeolusbot)
    {
        auto& mediator = ManagerMediator::getInstance();

        ::sc2::Units forces = mediator.GetUnitsFromRole(
            aeolusbot, constants::UnitRole::ATTACKING);

        // Defend the natural against bunker contains.
        const int baseToDefend = 1;
        const ::sc2::Point2D target =
            mediator.GetDefenseTarget(aeolusbot, baseToDefend);

        doGeneralMicro(aeolusbot, forces, target);

        auto voidRays = mediator.GetUnitsFromRole(
            aeolusbot, constants::UnitRole::DEFENSIVE_VOIDRAY);

        std::vector<::sc2::Point2D> starting_positions;
        for (const auto& voidray : voidRays)
        {
            starting_positions.push_back(voidray->pos);
        }
        auto voidRayTargets =
            mediator.GetUnitsInRange(aeolusbot, starting_positions, 16.0f);

        for (const auto* voidRay : voidRays)
        {
            auto voidray_behavior =
                std::make_unique<MicroBehavior>(voidRay);

            const bool lowShields =
                voidRay->shield < 0.1f * voidRay->shield_max;

            // Safety must have priority over ability use and attacking.
            if (lowShields)
            {
                voidray_behavior->AddBehavior(
                    std::make_unique<KeepUnitSafe>());
            }

            if (!voidRayTargets.empty())
            {
                auto in_attack_range =
                    mediator.GetUnitsInAtttackRange(
                        aeolusbot, voidRay, voidRayTargets);

                const ::sc2::Unit* enemy_target = nullptr;

                if (!in_attack_range.empty())
                {
                    enemy_target =
                        utils::PickAttackTarget(in_attack_range);

                    const auto& unit_data =
                        aeolusbot.Observation()
                        ->GetUnitTypeData()
                        .at(enemy_target->unit_type);

                    const bool is_armored =
                        std::find(
                            unit_data.attributes.begin(),
                            unit_data.attributes.end(),
                            ::sc2::Attribute::Armored)
                        != unit_data.attributes.end();

                    if (is_armored)
                    {
                        const auto availableAbilities =
                            aeolusbot.Query()
                            ->GetAbilitiesForUnit(voidRay)
                            .abilities;

                        const bool alignmentAvailable =
                            std::any_of(
                                availableAbilities.begin(),
                                availableAbilities.end(),
                                [](const auto& ability)
                                {
                                    return ability.ability_id ==
                                        ::sc2::ABILITY_ID::
                                        EFFECT_VOIDRAYPRISMATICALIGNMENT;
                                });

                        if (alignmentAvailable)
                        {
                            voidray_behavior->AddBehavior(
                                std::make_unique<UseAbility>(
                                    ::sc2::ABILITY_ID::
                                    EFFECT_VOIDRAYPRISMATICALIGNMENT));
                        }
                    }

                    // Attack either armored or unarmored targets.
                    // Use the exact target checked for Alignment.
                    voidray_behavior->AddBehavior(
                        std::make_unique<ShootTargetInRange>(
                            ::sc2::Units{ enemy_target }));
                }
                else
                {
                    enemy_target =
                        utils::PickAttackTarget(voidRayTargets);
                }

                if (!lowShields)
                {
                    voidray_behavior->AddBehavior(
                        std::make_unique<StutterUnitBack>(
                            enemy_target));
                }
            }

            voidray_behavior->AddBehavior(
                std::make_unique<PathToTarget>(target));

            voidray_behavior->AddBehavior(
                std::make_unique<AMove>(target));

            aeolusbot.RegisterBehavior(
                std::move(voidray_behavior));
        }
    }

    void MarauderRush::macro(AeolusBot& aeolusbot)
    {
        auto& mediator = ManagerMediator::getInstance();

        ::sc2::Units forces = mediator.GetUnitsFromRole(
            aeolusbot, constants::UnitRole::ATTACKING);

        bool hasVoidRays =
            !mediator.GetUnitsFromRole(
                aeolusbot,
                constants::UnitRole::DEFENSIVE_VOIDRAY).empty();

        std::map<::sc2::UNIT_TYPEID, float> army_comp = {
            {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}
        };

        for (const auto* unit : forces)
        {
            if (unit->unit_type ==
                ::sc2::UNIT_TYPEID::PROTOSS_VOIDRAY)
            {
                mediator.AssignRole(
                    aeolusbot,
                    unit,
                    constants::UnitRole::DEFENSIVE_VOIDRAY);

                hasVoidRays = true;
            }
        }

        auto ownStructure =
            mediator.GetAllOwnStructures(aeolusbot);

        // Build a shield battery at the natural.
        const bool hasShieldBattery = std::any_of(
            ownStructure.begin(),
            ownStructure.end(),
            [](const ::sc2::Unit* structure)
            {
                return structure->unit_type ==
                    ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY;
            });

        if (!hasShieldBattery &&
            mediator.GetNumberPending(
                aeolusbot,
                ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY) == 0)
        {
            aeolusbot.RegisterBehavior(
                std::make_unique<BuildStructure>(
                    ::sc2::UNIT_TYPEID::PROTOSS_SHIELDBATTERY,
                    1,
                    mediator.GetDefenseTarget(aeolusbot, 1)));
        }

        // build a stargate in the main, if not any:
        const bool hasStargate = std::any_of(
            ownStructure.begin(),
            ownStructure.end(),
            [](const ::sc2::Unit* structure)
            {
                return structure->unit_type ==
                    ::sc2::UNIT_TYPEID::PROTOSS_STARGATE;
            });

        if (!hasStargate &&
            mediator.GetNumberPending(
                aeolusbot,
                ::sc2::UNIT_TYPEID::PROTOSS_STARGATE) == 0)
        {
            aeolusbot.RegisterBehavior(
                std::make_unique<BuildStructure>(
                    ::sc2::UNIT_TYPEID::PROTOSS_STARGATE,
                    0,
                    false));
        }

        aeolusbot.RegisterBehavior(
            std::make_unique<Mining>());

        aeolusbot.RegisterBehavior(
            std::make_unique<Scout>());

        aeolusbot.RegisterBehavior(
            std::make_unique<AutoSupply>());

        bool hasAlmostReadyStargate = false;
        const ::sc2::Unit* readyStargate = nullptr;
        bool voidRayQueued = false;

        for (const auto* structure : ownStructure)
        {
            if (structure->unit_type !=
                ::sc2::UNIT_TYPEID::PROTOSS_STARGATE)
            {
                continue;
            }

            if (structure->build_progress > 0.90f)
            {
                hasAlmostReadyStargate = true;
            }

            if (structure->build_progress < 1.0f)
            {
                continue;
            }

            if (structure->orders.empty())
            {
                readyStargate = structure;
            }

            // Safe for empty orders; also checks the full queue.
            if (std::any_of(
                structure->orders.begin(),
                structure->orders.end(),
                [](const auto& order)
                {
                    return order.ability_id ==
                        ::sc2::ABILITY_ID::TRAIN_VOIDRAY;
                }))
            {
                voidRayQueued = true;
            }
        }

        if (!hasVoidRays && !voidRayQueued)
        {
            if (hasAlmostReadyStargate)
            {
                // Preserve the intended odd-loop scheduling.
                if (readyStargate &&
                    aeolusbot.Observation()->GetGameLoop() % 2 == 1)
                {
                    auto make_voidray_behavior =
                        std::make_unique<MicroBehavior>(
                            readyStargate);

                    make_voidray_behavior->AddBehavior(
                        std::make_unique<UseAbility>(
                            ::sc2::ABILITY_ID::TRAIN_VOIDRAY));

                    aeolusbot.RegisterBehavior(
                        std::move(make_voidray_behavior));
                }
            }
            else
            {
                aeolusbot.RegisterBehavior(
                    std::make_unique<SpawnController>(
                        army_comp));
            }

            aeolusbot.RegisterBehavior(
                std::make_unique<BuildWorkers>(22));

            aeolusbot.RegisterBehavior(
                std::make_unique<RepowerStructures>());
        }
        else
        {
            aeolusbot.RegisterBehavior(
                std::make_unique<BuildWorkers>(44));

            aeolusbot.RegisterBehavior(
                std::make_unique<SpawnController>(
                    army_comp));

            aeolusbot.RegisterBehavior(
                std::make_unique<ProductionController>(
                    army_comp));

            aeolusbot.RegisterBehavior(
                std::make_unique<ChronoController>());

            aeolusbot.RegisterBehavior(
                std::make_unique<RepowerStructures>());
        }

        // Preserve the intended safety-check scheduling and army selection.
        if (aeolusbot.Observation()->GetGameLoop() % 100 == 0)
        {
            auto ownAttacking = mediator.GetUnitsFromRole(
                aeolusbot, constants::UnitRole::ATTACKING);

            ::sc2::Units own_army;
            ::sc2::Units opponent_army;

            ::sc2::Units opponent_static_defenses =
                mediator.GetAllEnemyStaticDefenses(aeolusbot);

            for (const auto* unit : ownAttacking)
            {
                own_army.push_back(unit);
            }

            auto opponent_units =
                mediator.GetAllSeenEnemyUnits(aeolusbot);

            for (const auto* unit : opponent_units)
            {
                if (unit->unit_type !=
                    ::sc2::UNIT_TYPEID::PROTOSS_PROBE &&
                    unit->unit_type !=
                    ::sc2::UNIT_TYPEID::TERRAN_SCV &&
                    unit->unit_type !=
                    ::sc2::UNIT_TYPEID::ZERG_DRONE)
                {
                    opponent_army.push_back(unit);
                }
            }

            CombatSimulationResult combatResult =
                mediator.PredictEngagement(
                    aeolusbot,
                    own_army,
                    opponent_army,
                    opponent_static_defenses);

            const float armyRemainingDifference =
                combatResult.supplyPercentageRemaining -
                combatResult.enemySupplyPercentageRemaining;

            if (armyRemainingDifference > 0.7f ||
                aeolusbot.Observation()->GetFoodUsed() >= 195)
            {
                auto voidRays = mediator.GetUnitsFromRole(
                    aeolusbot,
                    constants::UnitRole::DEFENSIVE_VOIDRAY);

                for (const auto* voidRay : voidRays)
                {
                    mediator.AssignRole(
                        aeolusbot,
                        voidRay,
                        constants::UnitRole::ATTACKING);
                }

                auto scoutCandidate = mediator.SelectWorkerClosestTo(aeolusbot, mediator.GetEnemyNaturalPosition(aeolusbot));
                if (scoutCandidate) mediator.registerScout(aeolusbot, scoutCandidate.value(), { 2, 3, 4, 5 });

                aeolusbot.ChangeState(
                    MakeState<ForwardPressureState>());
            }
        }
    }

    void MarauderRush::OnEnter(AeolusBot& aeolusbot)
    {
#ifndef BUILD_FOR_LADDER
        aeolusbot.Actions()->SendChat(static_cast<std::string>(getName()));
#endif
        auto& mediator = ManagerMediator::getInstance();
        auto scoutCandidate = mediator.SelectWorkerClosestTo(aeolusbot, mediator.GetEnemyNaturalPosition(aeolusbot));
        if (scoutCandidate) mediator.registerScout(aeolusbot, scoutCandidate.value(), {2, 3, 4, 5});
    }
}