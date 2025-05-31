#pragma once  // Ensures the header is included only once

#include <sc2api/sc2_common.h>
#include "Bot.h"  // Include the base Bot class
#include "managers/hub.h"
#include "behavior_executor.h"
#include "states/bot_state.h"
#include <string>

#include "buildorder/buildorder.h"

namespace Aeolus
{
    class Behavior;

    // AeolusBot bot class inheriting from Bot
    class AeolusBot : public ::Bot {
    public:
        // Constructor
        AeolusBot(std::string opponent_id = "");

        // Destructor (optional, if you have cleanup tasks)
        ~AeolusBot();

        void RegisterBehavior(std::unique_ptr<Behavior> behavior);

        bool isBuildFinished() const;

        void setOpponentID(const std::string& opponent_id)
        {
            m_opponent_id = opponent_id;
        }

        std::map<::sc2::UNIT_TYPEID, float> getArmyComp();

        // change our current state to a new one
        void ChangeState(std::unique_ptr<BotState> new_state);

        int getMoveOutSupply();

    private:
        std::string m_opponent_id;

        std::unique_ptr<BuildOrderInterface> m_build_order;

        std::vector<::sc2::Point3D> m_expansion_locations;

        BuildOrderEnum m_build_order_enum;

        std::unique_ptr<BotState> m_currentState;

        // Executor for the bot's registered behaviors
        // Aeolus::BehaviorExecutor behavior_executor{};

        Hub manager_hub_;

        // select the strategy
        BuildOrderEnum _chooseBuildOrder();

        // based on the strategy, select the army comp
        std::map<::sc2::UNIT_TYPEID, float> m_armyComp;

        std::map<::sc2::UNIT_TYPEID, float> _chooseArmyComp();

        // based on the strategy, select the move out supply
        int _getMoveOutTiming();

        // move out timing
        int m_move_out_supply;

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
        void Micro();
        void PrepareUnits();

        // implement the build runner
        void ExecuteBuildOrder();
    };
}