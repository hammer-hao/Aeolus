// Aeolus main bot class

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_action.h>
#include <sc2lib/sc2_search.h>

#include <iostream>

#include "Aeolus.h"  // Include the header file
#include "managers/hub.h"

#include "managers/manager_mediator.h"
#include "behavior_executor.h"
#include "behaviors/macro_behaviors/mining.h"
#include "behaviors/macro_behaviors/build_workers.h"
#include "behaviors/macro_behaviors/build_structure.h"
#include "behaviors/macro_behaviors/expand.h"
#include "behaviors/macro_behaviors/build_geysers.h"
#include "behaviors/macro_behaviors/auto_supply.h"
#include "behaviors/macro_behaviors/production_controller.h"
#include "behaviors/macro_behaviors/spawn_controller.h"

#include "behaviors/micro_behaviors/micro_behavior.h"
#include "behaviors/micro_behaviors/path_to_target.h"
#include "behaviors/micro_behaviors/shoot_target_in_range.h"
#include "behaviors/micro_behaviors/keep_unit_safe.h"
#include "behaviors/micro_behaviors/a_move.h"
#include "behaviors/micro_behaviors/stutter_unit_back.h"

#include "build_order_executor.h"

#ifdef BUILD_WITH_RENDERER

#include "utils/feature_layer_utils.h"
#include "utils/unit_utils.h"
#include <sc2renderer/sc2_renderer.h>

#endif // BUILD_WITH_RENDERER


namespace Aeolus
{

    // Constructor
    AeolusBot::AeolusBot() {
        std::cout << "Aeolus bot initialized!" << std::endl;
        build_order_step = 0;
    }

    // Destructor (optional)
    AeolusBot::~AeolusBot() {
        std::cout << "Aeolus bot terminated!" << std::endl;
    }

    // Game start logic
    void AeolusBot::OnGameStart() {
        std::cout << "Aeolus: Game started!" << std::endl;
        //initialize feature layer
        #ifdef BUILD_WITH_RENDERER
        ::sc2::renderer::Initialize("Feature Layers", 50, 50, 2 * constants::DRAW_SIZE, 2 * constants::DRAW_SIZE);
        #endif
        manager_hub_ = Hub(*this);
        manager_hub_.Initialize();
    }

    // Game end logic
    void AeolusBot::OnGameEnd() {
        std::cout << "Aeolus: Game ended!" << std::endl;
        // Clean up or log stats
    }

    // Called every game step
    void AeolusBot::OnStep() {

        // std::cout << "Aeolus: Taking a step... " << std::endl;

        // Example: Get game loop information
        //uint32_t game_loop = observation->GetGameLoop();
        BeforeStep();

        // std::cout << "Aeolus: before step logic executed! " << std::endl;

        #ifdef BUILD_WITH_RENDERER
        
        ::sc2::ImageData pathing_grid = ManagerMediator::getInstance().GetDefaultGridData(*this);

        utils::DrawPathingGrid(pathing_grid);

        sc2::renderer::Render();
        
        #endif

        // std::cout << "Aeolus: Taking a step..." << std::endl;
        // std::cout << "DEBUG: About to call Macro()" << std::endl; // Add this

        Macro();

        // Micro our units
        ::sc2::Units forces = ManagerMediator::getInstance().GetUnitsFromRole(*this, constants::UnitRole::ATTACKING);
        if (!forces.empty()) Micro(forces, ManagerMediator::getInstance().GetExpansionLocations(*this).back());

        #ifndef BUILD_FOR_LADDER
        if (Observation()->GetGameLoop() % 100 == 0)
        {
            std::stringstream debugMessage;
            debugMessage << "Game Loop: " << Observation()->GetGameLoop() 
                << " Total Minerals Mined: " << Observation()->GetMinerals();
            Actions()->SendChat(debugMessage.str());
        }
        #endif
        // std::cout << "Aeolus: Taken a step!" << std::endl;
        // Call AfterStep at the end of each step
        AfterStep();

        // std::cout << "Aeolus: Executed all behaviors!" << std::endl;
    }

    // Handle unit creation
    void AeolusBot::OnUnitCreated(const ::sc2::Unit* unit_) {
        std::cout << "Aeolus:" << sc2::UnitTypeToName(unit_->unit_type) 
            << "(" << unit_->tag << ") was created" << std::endl;
        // Assign role based on unit type
        if (unit_->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_STALKER)
        {
            ManagerMediator::getInstance().AssignRole(*this, unit_, constants::UnitRole::ATTACKING);
        }
    }

    // Handle idle units
    void AeolusBot::OnUnitIdle(const ::sc2::Unit* unit_) {
        // std::cout << "Aeolus: " << sc2::UnitTypeToName(unit_->unit_type) 
        //     << "(" << unit_->tag << ") is idle" << std::endl;
        // Assign new orders to the idle unit
    }

    // Handle building construction completion
    void AeolusBot::OnBuildingConstructionComplete(const ::sc2::Unit* building_) {
        std::cout << "Aeolus: Building construction complete!" << std::endl;
    }

    // Handle unit destruction
    void AeolusBot::OnUnitDestroyed(const ::sc2::Unit* unit_) {
        std::cout << "Aeolus: Unit destroyed!" << std::endl;
        manager_hub_.OnUnitDestroyed(unit_);
    }

    // Handle upgrade completion
    void AeolusBot::OnUpgradeCompleted(::sc2::UpgradeID id_) {
        std::cout << "Aeolus: Upgrade completed!" << std::endl;
    }

    // Handle errors
    void AeolusBot::OnError(const std::vector<sc2::ClientError>& client_errors,
        const std::vector<std::string>& protocol_errors) {
        std::cerr << "Aeolus: Error encountered!" << std::endl;
        for (const auto& error : client_errors) {
            std::cerr << "Client Error: " << static_cast<int>(error) << std::endl;
        }
        for (const auto& error : protocol_errors) {
            std::cerr << "Protocol Error: " << error << std::endl;
        }
    }

    // Custom macro logic
    void AeolusBot::Macro() {
        // std::cout << "Aeolus: Macroing..." << std::endl;
        // Implement custom logic for gathering resources, expanding, etc.
        ExecuteBuildOrder();
        RegisterBehavior(std::make_unique<Mining>());
        RegisterBehavior(std::make_unique<BuildWorkers>(
            std::min(ManagerMediator::getInstance().GetOwnTownHalls(*this).size() * 22, static_cast<size_t>(86))
        ));
        RegisterBehavior(std::make_unique<Expand>());
        RegisterBehavior(std::make_unique<BuildGeysers>());
        RegisterBehavior(std::make_unique<AutoSupply>());
        RegisterBehavior(std::make_unique<ProductionController>(
            std::map<::sc2::UNIT_TYPEID, float>{{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}}
        )
        );
        RegisterBehavior(std::make_unique<SpawnController>(
            std::map<::sc2::UNIT_TYPEID, float>{{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}}
        )
        );

        if (Observation()->GetGameLoop() % 50 == 0)
        std::cout << "current gameloop: " << Observation()->GetGameLoop() << std::endl;
    }

    void AeolusBot::Micro(::sc2::Units units, ::sc2::Point2D target)
    {
        std::vector<::sc2::Point2D> starting_points;
        float search_radius = 15.0f;
        for (const auto& unit : units) starting_points.push_back(unit->pos);
        auto enemies_in_range = ManagerMediator::getInstance().GetEnemyUnitsInRangeMap(*this, 
            starting_points, search_radius);

        for (int i = 0; i < units.size(); ++i)
        {
            const ::sc2::Unit* unit = units[i];

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
                auto in_attack_range = ManagerMediator::getInstance().GetUnitsInAtttackRange(*this, unit, close_non_structures);
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
                    auto all_in_attack_range = ManagerMediator::getInstance().GetUnitsInAtttackRange(*this, unit, close_units);
                    if (!all_in_attack_range.empty())
                    {
                        combat_behavior->AddBehavior(
                            std::make_unique<ShootTargetInRange>(
                                all_in_attack_range
                            )
                        );
                    }
                }

                auto enemy_target = ::Aeolus::utils::PickAttackTarget(close_units);

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
            RegisterBehavior(std::move(combat_behavior));
        }
    }

    // Custom macro/economy management logic
    void AeolusBot::ManageEconomy() {
        std::cout << "Aeolus: Managing economy..." << std::endl;
        // Implement custom logic for gathering resources, expanding, etc.
    }

    // Custom army management logic
    void AeolusBot::ManageArmy() {
        std::cout << "Aeolus: Managing army..." << std::endl;
        // Implement custom logic for moving, attacking, defending, etc.
    }

    // Custom production management logic
    void AeolusBot::ManageProduction() {
        std::cout << "Aeolus: Managing production..." << std::endl;
        // Implement custom logic for training units, constructing buildings, etc.
    }
    void AeolusBot::BeforeStep()
    {

        // Logit to execute before each step begins
        PrepareUnits();
        manager_hub_.UpdateManagers(Observation()->GetGameLoop());

    }

    // Custom AfterStep logic
    void AeolusBot::AfterStep() {
        // Logic to execute after each step completes
        // TODO: Imprement behavior executioner!
        // std::cout << "Aeolus: calling behavior executor to execute behaviors... " << std::endl;
        BehaviorExecutor::GetInstance().ExecuteBehaviors(*this);
    }

    void AeolusBot::PrepareUnits() {
        // preparing units
        // std::cout << "Aeolus: Preparing Units" << std::endl;

        ::sc2::Units all_allied_units{ Observation()->GetUnits(::sc2::Unit::Alliance::Self)};

        // std::cout << "Aeolus: Trying to assign role to units..." << std::endl;

        // std::cout << "Aeolus: Processing " << all_allied_units.size() << "Units..." << std::endl;

        for (const sc2::Unit* unit : all_allied_units)
        {
            ManagerMediator::getInstance().CatchUnit(*this, unit);
        }
    }

    void AeolusBot::RegisterBehavior(std::unique_ptr<Behavior> behavior)
    {
        BehaviorExecutor::GetInstance().AddBehavior(std::move(behavior));
    }

    void AeolusBot::ExecuteBuildOrder()
    {
        BuildOrderExecutor::GetInstance().execute(*this);
    }
}
