message(STATUS "FetchContent: cpp-sc2")

# Dont build examples in the sc2api submodule.
set(BUILD_API_EXAMPLES OFF CACHE INTERNAL "" FORCE)

# Dont build tests in the sc2api submodule.
set(BUILD_API_TESTS OFF CACHE INTERNAL "" FORCE)

# dont build sc2renderer in the sc2api submodule.
set(BUILD_SC2_RENDERER OFF CACHE INTERNAL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    cpp_sc2
    GIT_REPOSITORY https://github.com/cpp_sc2/cpp-sc2.git
    GIT_TAG f4e8761ee6346c95aa0253d458df241631a4e778
)
FetchContent_MakeAvailable(cpp_sc2)

set_target_properties(sc2api PROPERTIES FOLDER cpp-sc2)
set_target_properties(sc2lib PROPERTIES FOLDER cpp-sc2)
set_target_properties(sc2protocol PROPERTIES FOLDER cpp-sc2)
set_target_properties(sc2utils PROPERTIES FOLDER cpp-sc2)

# Only set properties for sc2renderer if it is being built
if (BUILD_WITH_RENDERER)
    # Get the source directory of cpp-sc2
    set(CPP_SC2_DIR ${cpp-sc2_SOURCE_DIR})

    # Verify the directory
    message(STATUS "CPP_SC2_DIR: ${CPP_SC2_DIR}")

    set(SC2_RENDERER_DIR "${CPP_SC2_DIR}/src/sc2renderer")
    message(STATUS "SC2_RENDERER_DIR: ${SC2_RENDERER_DIR}")

    file(GLOB SC2_RENDERER_SOURCES
        "${SC2_RENDERER_DIR}/*.cc"
        "${SC2_RENDERER_DIR}/*.h"
    )

    message(STATUS "SC2_RENDERER_SOURCES: ${SC2_RENDERER_SOURCES}")
    # set_target_properties(sc2renderer PROPERTIES FOLDER cpp-sc2)
endif()
