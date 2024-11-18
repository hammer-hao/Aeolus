#pragma once  // Ensures the header is included only once

#include <sc2api/sc2_common.h>
#include "Bot.h"  // Include the base Bot class
#include "behavior_executor.h"

namespace Aeolus
{
    class Behavior;

    // AeolusBot bot class inheriting from Bot
    class AeolusBot : public ::Bot {
    public:
        // Constructor
        AeolusBot();

        // Destructor (optional, if you have cleanup tasks)
        ~AeolusBot();

    private:

        std::vector<::sc2::Point3D> m_expansion_locations;

        // Executor for the bot's registered behaviors
        // Aeolus::BehaviorExecutor behavior_executor{};


        // Override Bot's methods to implement your custom behavior
        void OnGameStart() final override;
        void OnGameEnd() final override;
        void OnStep() final override;
        void OnUnitCreated(const ::sc2::Unit* unit_) final override;
        void OnUnitIdle(const ::sc2::Unit* unit_) final override;
        void OnBuildingConstructionComplete(const ::sc2::Unit* building_) final override;
        void OnUnitDestroyed(const ::sc2::Unit* unit_) final override;
        void OnUpgradeCompleted(::sc2::UpgradeID id_) final override;
        void OnError(const std::vector<::sc2::ClientError>& client_errors,
            const std::vector<std::string>& protocol_errors = {}) final override;

        // Custom method for post-step logic
        void BeforeStep();
        void AfterStep();  // Add AfterStep method

        // Add any custom methods or members you need
        void ManageEconomy();    // Custom macro/economy logic
        void ManageArmy();       // Custom army management logic
        void ManageProduction(); // Custom production logic
        void Macro();
        void PrepareUnits();
        void RegisterBehavior(std::unique_ptr<Behavior> behavior);

        bool m_assigned_initial_workers = false;
    };

}