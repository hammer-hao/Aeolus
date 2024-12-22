# The MIT License (MIT)
#
# Copyright (c) 2021-2024 Alexander Kurbatov

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Group files into folders

source_group("Source Files" FILES main.cpp bot.cpp constants.cpp behavior_executor.cpp Aeolus.cpp)
source_group("Header Files" FILES Bot.h constants.h behavior_executor.h Aeolus.h)

source_group("Source Files\\behaviors\\macro_behaviors" FILES behaviors/macro_behaviors/mining.cpp)
source_group("Header Files\\behaviors" FILES behaviors/behavior.h)
source_group("Header Files\\behaviors\\macro_behaviors" FILES behaviors/macro_behaviors/macro_behavior.h behaviors/macro_behaviors/mining.h)

source_group("Source Files\\Managers" FILES 
    managers/hub.cpp
    managers/manager_mediator.cpp
    managers/path_manager.cpp
    managers/resource_manager.cpp
    managers/unit_filter_manager.cpp
    managers/unit_property_manager.cpp
    managers/unit_role_manager.cpp)

source_group("Header Files\\Managers" FILES 
    managers/hub.h
    managers/manager.h
    managers/manager_mediator.h
    managers/path_manager.h
    managers/resource_manager.h
    managers/unit_filter_manager.h
    managers/unit_property_manager.h
    managers/unit_role_manager.h)

source_group("Source Files\\Pathing" FILES
    pathing/grid.cpp
    pathing/map_data.cpp)

source_group("Header Files\\Pathing" FILES
    pathing/grid.h
    pathing/map_data.h
    pathing/weight_costs.h)

source_group("Source Files\\utils" FILES
    utils/feature_layer_utils.cpp
    utils/game_utils.cpp
    utils/grid_utils.cpp
    utils/position_utils.cpp
    utils/unit_utils.cpp)

source_group("Header Files\\utils" FILES
    utils/feature_layer_utils.h
    utils/game_utils.h
    utils/grid_utils.h
    utils/position_utils.h
    utils/unit_utils.h)

set(bot_sources
    Aeolus.cpp
    main.cpp
    Bot.cpp
    constants.cpp
    behavior_executor.cpp
    behaviors/macro_behaviors/mining.cpp
    managers/hub.cpp
    managers/manager_mediator.cpp
    managers/path_manager.cpp
    managers/resource_manager.cpp
    managers/unit_filter_manager.cpp
    managers/unit_property_manager.cpp
    managers/unit_role_manager.cpp
    pathing/grid.cpp
    pathing/map_data.cpp
    utils/feature_layer_utils.cpp
    utils/game_utils.cpp
    utils/grid_utils.cpp
    utils/position_utils.cpp
    utils/unit_utils.cpp)

set(bot_headers
    Aeolus.h
    Bot.h
    constants.h
    behavior_executor.h
    behaviors/macro_behaviors/macro_behavior.h behaviors/macro_behaviors/mining.h
    behaviors/behavior.h
    managers/hub.h
    managers/manager.h
    managers/manager_mediator.h
    managers/path_manager.h
    managers/resource_manager.h
    managers/unit_filter_manager.h
    managers/unit_property_manager.h
    managers/unit_role_manager.h
    pathing/grid.h
    pathing/map_data.h
    pathing/weight_costs.h
    utils/feature_layer_utils.h
    utils/game_utils.h
    utils/grid_utils.h
    utils/position_utils.h
    utils/unit_utils.h)

include(FetchContent)

add_executable(Aeolus ${bot_sources} ${bot_headers})

option(BUILD_FOR_LADDER "Build Aeolus for ladder mode" OFF)

if (BUILD_FOR_LADDER)
    target_compile_definitions(Aeolus PRIVATE BUILD_FOR_LADDER)
endif ()

if (BUILD_WITH_RENDERER)
    target_compile_definitions(Aeolus PRIVATE BUILD_WITH_RENDERER)
    # Include SDL2
    if (NOT SDL2_INCLUDE_DIR)
        message(FATAL_ERROR "SDL2 fetch failed: SDL2_INCLUDE_DIR not set.")
    endif ()

    target_include_directories(Aeolus PUBLIC ${SDL2_INCLUDE_DIR})
    target_link_libraries(Aeolus PRIVATE ${SDL2_LIBRARIES})

    # include sc2renderer
    add_library(sc2renderer ${SC2_RENDERER_SOURCES})
    target_include_directories(sc2renderer PUBLIC ${SC2_RENDERER_DIR})

    target_include_directories(sc2renderer PRIVATE ${SDL2_INCLUDE_DIR})
    target_link_libraries(sc2renderer PRIVATE ${SDL2_LIBRARIES})

    target_link_libraries(Aeolus PRIVATE sc2renderer)
endif ()

if (MSVC)
    target_compile_options(Aeolus PRIVATE /W4 /EHsc)
else ()
    target_compile_options(Aeolus PRIVATE -Wall -Wextra -pedantic)
endif ()

target_link_libraries(Aeolus PRIVATE cpp_sc2)
target_link_libraries(Aeolus PRIVATE eigen)

if (MINGW)
    target_link_libraries(Aeolus PRIVATE ssp)
elseif (APPLE)
    target_link_libraries(Aeolus PRIVATE "-framework Carbon")
# Building on Linux
elseif (UNIX AND NOT APPLE)
    target_link_libraries(Aeolus PRIVATE pthread dl)
endif ()
