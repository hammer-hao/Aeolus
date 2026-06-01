// Aeolus main bot class

#include <sc2api/sc2_common.h>
#include <sc2api/sc2_unit.h>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_action.h>
#include <sc2lib/sc2_search.h>
#include <sc2api/sc2_typeenums.h>

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
#include "behaviors/macro_behaviors/upgrades_controller.h"
#include "behaviors/macro_behaviors/chrono_controller.h"
#include "behaviors/macro_behaviors/repower_structures.h"

#include "behaviors/micro_behaviors/micro_behavior.h"
#include "behaviors/micro_behaviors/path_to_target.h"
#include "behaviors/micro_behaviors/shoot_target_in_range.h"
#include "behaviors/micro_behaviors/keep_unit_safe.h"
#include "behaviors/micro_behaviors/a_move.h"
#include "behaviors/micro_behaviors/stutter_unit_back.h"
#include "behaviors/micro_behaviors/unload.h"
#include "behaviors/micro_behaviors/pick_unit_up.h"

#include "states/bot_state.h"
#include "states/build_order_state.h"
#include "states/forward_pressure.h"
#include "states/consolidate.h"

#include "utils/unit_utils.h"
#include "utils/position_utils.h"
#include "utils/file_io_utils.h"

#include "buildorder/buildorderfactory.h"

#ifdef BUILD_WITH_RENDERER

#include "utils/feature_layer_utils.h"
#include <sc2renderer/sc2_renderer.h>

#endif // BUILD_WITH_RENDERER


namespace Aeolus
{

    // Constructor
    AeolusBot::AeolusBot(std::string opponent_id) : m_opponent_id(opponent_id), m_won_game(true)
    {
        m_build_order_enum = _chooseBuildOrder();

        m_move_out_supply = _getMoveOutTiming();

        std::cout << "Aeolus bot initialized!" << std::endl;
    }

    // Destructor (optional)
    AeolusBot::~AeolusBot() {
        std::cout << "Aeolus bot terminated!" << std::endl;
        utils::recordMatchResult(m_opponent_id, buildOrderToString(m_build_order_enum), m_won_game);
    }

    BuildOrderEnum AeolusBot::_chooseBuildOrder()
    {

        m_build_order_enum = BuildOrderEnum::STALKER_STARGATE;
        m_armyComp = _chooseArmyComp();
        return m_build_order_enum;
    }

    std::map<::sc2::UNIT_TYPEID, float> AeolusBot::_chooseArmyComp()
    {
        std::cout << "Build order: " << buildOrderToString(m_build_order_enum) << std::endl;

        switch (m_build_order_enum)
        {
        case (BuildOrderEnum::MACRO_STALKERS):
        {
            return std::map<::sc2::UNIT_TYPEID, float>{{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}};
        }
        case (BuildOrderEnum::BLINK_STALKERS):
        {
            return std::map<::sc2::UNIT_TYPEID, float>{{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}};
        }
        case (BuildOrderEnum::FORGE_EXPAND):
        {
            return std::map<::sc2::UNIT_TYPEID, float>{{::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}};
        }
        case (BuildOrderEnum::STALKER_STARGATE):
        {
            return std::map < ::sc2::UNIT_TYPEID, float>
            {
                {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 1.0f}
            };
        }
        case (BuildOrderEnum::STALKER_IMMORTAL):
        {
            return std::map < ::sc2::UNIT_TYPEID, float>
            {
                {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 0.7f},
                {::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, 0.3f }
            };
        }
        case (BuildOrderEnum::MASS_ROBO):
        {
            std::cout << "[_chooseArmyComp] in MASS_ROBO branch" << std::endl;
            auto ret = std::map<::sc2::UNIT_TYPEID, float>{
              {::sc2::UNIT_TYPEID::PROTOSS_COLOSSUS, 0.2f},
              {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 0.7f},
              {::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, 0.1f},
            };
            std::cout << "[_chooseArmyComp] about to return, map size = "
                << ret.size() << std::endl;
            return ret;
        }
        default:
        {
            std::cout << "Unknown build order: " << buildOrderToString(m_build_order_enum) << std::endl;
            return std::map < ::sc2::UNIT_TYPEID, float>
            {
                {::sc2::UNIT_TYPEID::PROTOSS_STALKER, 0.7f},
                { ::sc2::UNIT_TYPEID::PROTOSS_IMMORTAL, 0.3f }
            };
        }
        }
    }

    // get move out time from strategy (number of units needed for move-out)
    int AeolusBot::_getMoveOutTiming()
    {
        switch (m_build_order_enum)
        {
        case (BuildOrderEnum::MACRO_STALKERS):
        {
            return 2;
        }
        case (BuildOrderEnum::STALKER_IMMORTAL):
        {
            return 3;
        }
        case (BuildOrderEnum::BLINK_STALKERS):
        {
            return 6;
        }
        case (BuildOrderEnum::FORGE_EXPAND):
        {
            return 8;
        }
        case (BuildOrderEnum::MASS_ROBO):
        {
            return 6;
        }
        case (BuildOrderEnum::STALKER_STARGATE):
        {
            return 10;
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
        buildOrderTag << buildOrderToString(m_build_order_enum);
        Actions()->SendChat(buildOrderTag.str());

        const auto own_id = Observation()->GetPlayerID();

        ::sc2::Race opponentRace = ::sc2::Race::Random;
        for (const auto& player : Observation()->GetGameInfo().player_info) {
            if (player.player_id != own_id) {
                opponentRace = player.race_requested;
            }
        }
        m_build_order = BuildOrderFactory::makeBuildOrder(*this, m_build_order_enum, opponentRace);
        std::stringstream opponentRaceTag;
        opponentRaceTag << "Tag:";
        if (opponentRace == ::sc2::Race::Protoss) {
            opponentRaceTag << "opponent: Protoss";
        } else if (opponentRace == ::sc2::Race::Terran) {
            opponentRaceTag << "opponent: Terran";
        } else if (opponentRace == ::sc2::Race::Zerg) {
            opponentRaceTag << "opponent: Zerg";
        } else if (opponentRace == ::sc2::Race::Random) {
            opponentRaceTag << "opponent: Random";
        }
        Actions()->SendChat(opponentRaceTag.str());

        // set state initially to consolidate
        ChangeState(MakeState<BuildOrderState>());
    }

    // Game end logic
    void AeolusBot::OnGameEnd() {
        std::cout << "Aeolus: Game ended!" << std::endl;
    }
    // Called every game step
    void AeolusBot::OnStep() {

        auto& mediator = ManagerMediator::getInstance();

        // std::cout << "Aeolus: Taking a step... " << std::endl;

        // Example: Get game loop information
        //uint32_t game_loop = observation->GetGameLoop();
        BeforeStep();

        // std::cout << "Aeolus: before step logic executed! " << std::endl;

        #ifdef BUILD_WITH_RENDERER
        
        ::sc2::ImageData pathing_grid = mediator.GetDefaultGridData(*this);

        utils::DrawPathingGrid(pathing_grid);

        sc2::renderer::Render();
        
        #endif

        // std::cout << "Aeolus: Taking a step..." << std::endl;
        // std::cout << "DEBUG: About to call Macro()" << std::endl; // Add this

        Macro();
        Micro();

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
        // 
        // Call AfterStep at the end of each step
        AfterStep();
    }

    // Handle unit creation
    void AeolusBot::OnUnitCreated(const ::sc2::Unit* unit_) {
        std::cout << "Aeolus:" << sc2::UnitTypeToName(unit_->unit_type) 
            << "(" << unit_->tag << ") was created" << std::endl;

        manager_hub_.OnUnitCreated(unit_);
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
        manager_hub_.OnUnitDestroyed(unit_);
        m_currentState->OnUnitDestroyed(*this, unit_);

        // if we have only one building left, record as loss:
        int buildings_left = ManagerMediator::getInstance().GetAllOwnStructures(*this).size();
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
        m_currentState->macro(*this);
    }

    void AeolusBot::Micro()
    {
        m_currentState->micro(*this);
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
        m_build_order->execute();
    }

    void AeolusBot::ChangeState(std::unique_ptr<BotState> new_state)
    {
        if (m_currentState) m_currentState->OnExit();
        m_currentState = std::move(new_state);
        if (m_currentState) m_currentState->OnEnter(*this);
    }

    bool AeolusBot::isBuildFinished() const
    {
        return m_build_order->isFinished();
    }

    std::map<::sc2::UNIT_TYPEID, float> AeolusBot::getArmyComp()
    {
        return m_armyComp;
    }

    int AeolusBot::getMoveOutSupply()
    {
        return m_move_out_supply;
    }
}
