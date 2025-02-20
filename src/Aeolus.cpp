// Aeolus main bot class

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_action.h>
#include <sc2lib/sc2_search.h>

#include <iostream>
#include <random>

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
#include "build_order_enum.h"
#include "utils/unit_utils.h"
#include "utils/position_utils.h"
#include "utils/file_io_utils.h"
#include "utils/strategy_utils.h"

#ifdef BUILD_WITH_RENDERER

#include "utils/feature_layer_utils.h"
#include <sc2renderer/sc2_renderer.h>
#include "build_order_executor.h"

#endif // BUILD_WITH_RENDERER


namespace Aeolus
{

    // Constructor
    AeolusBot::AeolusBot(std::string opponent_id) : m_opponent_id(opponent_id), m_build_order_executor(_chooseBuildOrder()), m_won_game(true)
    {
        std::cout << "Aeolus bot initialized!" << std::endl;
        m_current_base_target = 0;
    }

    // Destructor (optional)
    AeolusBot::~AeolusBot() {
        std::cout << "Aeolus bot terminated!" << std::endl;
        utils::recordMatchResult(m_opponent_id, buildOrderToString(m_build_order), m_won_game);
    }

    BuildOrderEnum AeolusBot::_chooseBuildOrder()
    {
        std::cout << "Aeolust: choosing build order..." << std::endl;
        //static std::random_device rd;
        //static std::mt19937 gen(rd());
        //std::uniform_int_distribution<int> dist(0, static_cast<int>(BuildOrderEnum::COUNT) - 1);
#ifdef BUILD_FOR_LADDER
        std::string opponent = m_opponent_id;
#else
        std::string opponent = "";
#endif // BUILD_FOR_LADDER
        std::cout << "Opponent id: " << opponent << std::endl;
        BuildOrderEnum result = utils::chooseBestStrateGyFromHistory(utils::getMatchesForOpponent(opponent));
        m_build_order = result;
        return result;
    }

    std::map<::sc2::UNIT_TYPEID, float> AeolusBot::_chooseArmyComp()
    {
        switch (m_build_order)
        {
        case (BuildOrderEnum::MACRO_STALKERS):
        {
            return std::map<::sc2::UNIT_TYPEID, float>{{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}};
        }
        case (BuildOrderEnum::STALKER_IMMORTAL):
        {
            return std::map < ::sc2::UNIT_TYPEID, float>
            {
                {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 0.7f},
                {::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, 0.3f }
            };
        }
        }
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

        // tag the replay with chosen build:
        std::stringstream buildOrderTag;
        buildOrderTag << "Tag:";
        if (m_build_order == BuildOrderEnum::MACRO_STALKERS) buildOrderTag << "MACRO_STALKERS";
        if (m_build_order == BuildOrderEnum::STALKER_IMMORTAL) buildOrderTag << "STALKER_IMMORTAL";
        Actions()->SendChat(buildOrderTag.str());
    }

    // Game end logic
    void AeolusBot::OnGameEnd() {
        std::cout << "Aeolus: Game ended!" << std::endl;
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
        if (!forces.empty()) Micro(forces, CalculateTarget());

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
        if (unit_->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_STALKER
            || unit_->unit_type == ::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL)
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

        // if we have only one building left, record as loss:
        int buildings_left = ManagerMediator::getInstance().GetAllOwnStructures(*this).size();

        std::cout << "Aeolus: there are " << buildings_left << " buildings left" << std::endl;
        if (buildings_left <= 2)
        {
            m_won_game = false;
        }
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
            std::min(ManagerMediator::getInstance().GetOwnReadyTownHalls(*this).size() * 22, static_cast<size_t>(86))
        ));
        RegisterBehavior(std::make_unique<Expand>());
        RegisterBehavior(std::make_unique<AutoSupply>());
        RegisterBehavior(std::make_unique<ProductionController>(_chooseArmyComp()));
        RegisterBehavior(std::make_unique<SpawnController>(_chooseArmyComp()));

        if (m_build_order_executor.isDone())
        {
            RegisterBehavior(std::make_unique<BuildGeysers>());
        }

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
            RegisterBehavior(std::move(combat_behavior));
        }
    }

    // calculate the next target to attack
    ::sc2::Point2D AeolusBot::CalculateTarget()
    {
        auto enemy_structures = ManagerMediator::getInstance().GetAllEnemyStructures(*this);
        ::sc2::Units filtered_structures;
        for (const auto& structure : enemy_structures)
        {
            if (structure->unit_type != ::sc2::UNIT_TYPEID::ZERG_CREEPTUMOR
                && structure->unit_type != ::sc2::UNIT_TYPEID::ZERG_CREEPTUMORBURROWED
                && structure->unit_type != ::sc2::UNIT_TYPEID::ZERG_CREEPTUMORQUEEN)
                filtered_structures.push_back(structure);
        }
        if (!filtered_structures.empty())
        {
            return utils::GetClosestUnitTo(Observation()->GetStartLocation(), filtered_structures)->pos;
        }
        else if (Observation()->GetGameLoop() / 22.4f < 240.0f) 
            return ManagerMediator::getInstance().GetExpansionLocations(*this).back();
        else
        {
            auto targets = ManagerMediator::getInstance().GetExpansionLocations(*this);
            if (Observation()->GetVisibility(targets[m_current_base_target]) == ::sc2::Visibility::Visible)
            {
                if (m_current_base_target == 0) m_current_base_target = (targets.size() - 1);
                else if (m_current_base_target == targets.size() - 1) m_current_base_target = 1;
                else m_current_base_target = (m_current_base_target + 1) % targets.size();
            }
            return targets[m_current_base_target];
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
        m_build_order_executor.execute(*this);
    }
}
