#pragma once  // Ensures the header is included only once

#include <sc2api/sc2_common.h>
#include "Bot.h"  // Include the base Bot class
#include "managers/hub.h"
#include "behavior_executor.h"
#include "build_order_executor.h"

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

        void RegisterBehavior(std::unique_ptr<Behavior> behavior);

        void setOpponentID(const std::string& opponent_id)
        {
            m_opponent_id = opponent_id;
        }

    private:

        BuildOrderExecutor m_build_order_executor;

        std::vector<::sc2::Point3D> m_expansion_locations;

        BuildOrderEnum m_build_order;

        // Executor for the bot's registered behaviors
        // Aeolus::BehaviorExecutor behavior_executor{};

        Hub manager_hub_;

        // select the strategy
        BuildOrderEnum _chooseBuildOrder();

        // based on the strategy, select the army comp
        std::map<::sc2::UNIT_TYPEID, float> _chooseArmyComp();

        std::string m_opponent_id;

        // keep track of lose condition
        bool m_won_game;

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

        // implement the build runner
        void ExecuteBuildOrder();

        // micro units
        void Micro(::sc2::Units units, ::sc2::Point2D target);

        int m_current_base_target;

        // calculate the next target to attack
        ::sc2::Point2D CalculateTarget();
    };

}