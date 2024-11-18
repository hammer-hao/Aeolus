// The MIT License (MIT)
//
// Copyright (c) 2021-2024 Alexander Kurbatov

#pragma once

#include <sc2api/sc2_agent.h>

// The main bot class.
// Bot.h (Modified)
struct Bot : public sc2::Agent {
    Bot() = default;

    // Methods without 'final' so they can be overridden.
    virtual void OnGameStart();
    virtual void OnGameEnd();
    virtual void OnStep();
    virtual void OnBuildingConstructionComplete(const sc2::Unit* building_);
    virtual void OnUnitCreated(const sc2::Unit* unit_);
    virtual void OnUnitIdle(const sc2::Unit* unit_);
    virtual void OnUnitDestroyed(const sc2::Unit* unit_);
    virtual void OnUpgradeCompleted(sc2::UpgradeID id_);
    virtual void OnError(const std::vector<sc2::ClientError>& client_errors,
        const std::vector<std::string>& protocol_errors = {});
};