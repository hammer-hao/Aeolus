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

#ifdef BUILD_WITH_RENDERER

#include "utils/feature_layer_utils.h"
#include <sc2renderer/sc2_renderer.h>

#endif // BUILD_WITH_RENDERER


namespace Aeolus
{

    // Constructor
    AeolusBot::AeolusBot() {
        std::cout << "Aeolus bot initialized!" << std::endl;
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

        // Initialize resources, strategies, etc.
        manager_hub_ = Hub(*this);
        // ManagerMediator::getInstance().Populate(*this);

        // register the expansion locations here
        m_expansion_locations = ::sc2::search::CalculateExpansionLocations(Observation(), Query());

        ManagerMediator::getInstance().CalculateMineralGatheringPoints(*this, m_expansion_locations);

        ::sc2::Units destructables = ManagerMediator::getInstance().GetAllDestructables(*this);

        std::stringstream debugMessage;
        debugMessage << "Got Destructables: " << destructables.size();
        Actions()->SendChat(debugMessage.str());

        ::sc2::ImageData default_grid = ManagerMediator::getInstance().GetDefaultGridData(*this);
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
        Macro();

        if (Observation()->GetGameLoop() % 100 == 0)
        {
            std::stringstream debugMessage;
            debugMessage << "Game Loop: " << Observation()->GetGameLoop() 
                << " Total Minerals Mined: " << Observation()->GetMinerals();
            Actions()->SendChat(debugMessage.str());
        }
        // Call AfterStep at the end of each step
        AfterStep();
    }

    // Handle unit creation
    void AeolusBot::OnUnitCreated(const ::sc2::Unit* unit_) {
        std::cout << "Aeolus:" << sc2::UnitTypeToName(unit_->unit_type) 
            << "(" << unit_->tag << ") was created" << std::endl;
    }

    // Handle idle units
    void AeolusBot::OnUnitIdle(const ::sc2::Unit* unit_) {
        std::cout << "Aeolus: " << sc2::UnitTypeToName(unit_->unit_type) 
            << "(" << unit_->tag << ") is idle" << std::endl;
        // Assign new orders to the idle unit
    }

    // Handle building construction completion
    void AeolusBot::OnBuildingConstructionComplete(const ::sc2::Unit* building_) {
        std::cout << "Aeolus: Building construction complete!" << std::endl;
    }

    // Handle unit destruction
    void AeolusBot::OnUnitDestroyed(const ::sc2::Unit* unit_) {
        std::cout << "Aeolus: Unit destroyed!" << std::endl;
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
        RegisterBehavior(std::make_unique<Mining>());

        if (Observation()->GetGameLoop() % 50 == 0)
        std::cout << "current gameloop: " << Observation()->GetGameLoop() << std::endl;
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

        if (!m_assigned_initial_workers)
        {
            std::cout << "<<< Aeolus: Assigning (initial) workers >>>" << std::endl;
            ManagerMediator::getInstance().AssignInitialWorkers(*this);
            m_assigned_initial_workers = true;
        }
        
    }

    void AeolusBot::RegisterBehavior(std::unique_ptr<Behavior> behavior)
    {
        BehaviorExecutor::GetInstance().AddBehavior(std::move(behavior));
    }
}
