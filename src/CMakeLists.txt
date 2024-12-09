# The MIT License (MIT)
#
# Copyright (c) 2021-2024 Alexander Kurbatov

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(bot_sources
    main.cpp
    Bot.cpp)

include(FetchContent)

add_executable(Aeolus ${bot_sources})

option(BUILD_FOR_LADDER "Build Aeolus for ladder mode" OFF)

if (BUILD_FOR_LADDER)
    target_compile_definitions(Aeolus PRIVATE BUILD_FOR_LADDER)
endif ()

if (MSVC)
    target_compile_options(Aeolus PRIVATE /W4 /EHsc)
else ()
    target_compile_options(Aeolus PRIVATE -Wall -Wextra -pedantic)
endif ()

target_link_libraries(Aeolus PRIVATE cpp_sc2)
target_link_libraries(Aeolus PRIVATE eigen)

# Include SDL2
if (NOT SDL2_INCLUDE_DIR)
    message(FATAL_ERROR "SDL2 fetch failed: SDL2_INCLUDE_DIR not set.")
endif ()
target_include_directories(Aeolus PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(Aeolus PRIVATE ${SDL2_LIBRARIES})

if (MINGW)
    target_link_libraries(Aeolus PRIVATE ssp)
elseif (APPLE)
    target_link_libraries(Aeolus PRIVATE "-framework Carbon")
# Building on Linux
elseif (UNIX AND NOT APPLE)
    target_link_libraries(Aeolus PRIVATE pthread dl)
endif ()
