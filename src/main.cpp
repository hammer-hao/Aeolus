// The MIT License (MIT)
//
// Copyright (c) 2021-2024 Alexander Kurbatov

#include "Bot.h"
#include "Aeolus.h"

#include <sc2api/sc2_coordinator.h>
#include <sc2api/sc2_gametypes.h>
#include <sc2utils/sc2_arg_parser.h>

#include <iostream>
#include "constants.h"

#ifdef BUILD_FOR_LADDER
namespace
{

struct Options
{
    Options(): GamePort(0), StartPort(0)
    {}

    int32_t GamePort;
    int32_t StartPort;
    std::string ServerAddress;
    std::string OpponentId;
};

void ParseArguments(int argc, char* argv[], Options* options_)
{
    sc2::ArgParser arg_parser(argv[0]);
    arg_parser.AddOptions(
        {
            {"-g", "--GamePort", "Port of client to connect to", false},
            {"-o", "--StartPort", "Starting server port", false},
            {"-l", "--LadderServer", "Ladder server address", false},
            {"-x", "--OpponentId", "PlayerId of opponent", false},
        });

    arg_parser.Parse(argc, argv);

    // GamePort
    std::string GamePortStr;
    if (arg_parser.Get("GamePort", GamePortStr)) {
        options_->GamePort = atoi(GamePortStr.c_str());
        std::cout << "GamePort: " << options_->GamePort << std::endl;
    }
    else {
        std::cout << "GamePort not provided or invalid. Using default value: " << options_->GamePort << std::endl;
    }

    // StartPort
    std::string StartPortStr;
    if (arg_parser.Get("StartPort", StartPortStr)) {
        options_->StartPort = atoi(StartPortStr.c_str());
        std::cout << "StartPort: " << options_->StartPort << std::endl;
    }
    else {
        std::cout << "StartPort not provided or invalid. Using default value: " << options_->StartPort << std::endl;
    }

    // OpponentId
    std::string OpponentId;
    if (arg_parser.Get("OpponentId", OpponentId)) {
        options_->OpponentId = OpponentId;
        std::cout << "OpponentId: " << options_->OpponentId << std::endl;
    }
    else {
        std::cout << "OpponentId not provided. Defaulting to empty string." << std::endl;
    }

    // LadderServer
    if (arg_parser.Get("LadderServer", options_->ServerAddress)) {
        std::cout << "LadderServer: " << options_->ServerAddress << std::endl;
    }
    else {
        std::cout << "LadderServer not provided. Defaulting to empty string." << std::endl;
    }

}

}  // namespace

int main(int argc, char* argv[])
{
    Options options;
    ParseArguments(argc, argv, &options);

    sc2::Coordinator coordinator;
    Aeolus::AeolusBot aeolus_bot(options.OpponentId);

    size_t num_agents = 2;
    coordinator.SetParticipants({ CreateParticipant(sc2::Race::Protoss, &aeolus_bot, "Aeolus") });

    std::cout << "Connecting to port " << options.GamePort << std::endl;

    std::cout << "Connecting to SC2 Client..." << std::endl;
    coordinator.Connect(options.GamePort);
    std::cout << "Connected to SC2 Client! " << std::endl;

    std::cout << "Setting up ports... " << std::endl;
    coordinator.SetupPorts(num_agents, options.StartPort, false);
    std::cout << "Port setup completed! " << std::endl;

    // NB (alkurbatov): Increase speed of steps processing.
    // Disables ability to control your bot during game.
    // Recommended for competitions.
    coordinator.SetRawAffectsSelection(true);

    std::cout << "Joining game..." << std::endl;
    coordinator.JoinGame();
    coordinator.SetTimeoutMS(10000);
    std::cout << "Successfully joined game" << std::endl;

    while (coordinator.Update())
    {}

    return 0;
}

#else

#include <sc2renderer/sc2_renderer.h>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Provide either name of the map file or path to it!" << std::endl;
        return 1;
    }

    sc2::Coordinator coordinator;
    coordinator.LoadSettings(argc, argv);

    // NOTE: Uncomment to start the game in full screen mode.
    // coordinator.SetFullScreen(true);

    // NOTE: Uncomment to play at normal speed.
    // coordinator.SetRealtime(true);

    // Create an Aeodus bot instance
    Aeolus::AeolusBot aeolus_bot;

    sc2::FeatureLayerSettings settings(Aeolus::constants::CAMERA_WIDTH, Aeolus::constants::FEATURE_LAYER_SIZE, Aeolus::constants::FEATURE_LAYER_SIZE,
        Aeolus::constants::FEATURE_LAYER_SIZE, Aeolus::constants::FEATURE_LAYER_SIZE);

    coordinator.SetFeatureLayers(settings);

    coordinator.SetParticipants(
        {
            CreateParticipant(sc2::Race::Protoss, &aeolus_bot, "Aeolus"),
            CreateComputer(
                sc2::Race::Terran,
                sc2::Difficulty::Easy,
                sc2::AIBuild::Macro,
                "BOYNEXTDOOR"
                )
        });

    coordinator.LaunchStarcraft();
    coordinator.StartGame(argv[1]);

    while (coordinator.Update())
    {}

    return 0;
}

#endif
