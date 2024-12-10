# Fetch SDL2 Library
include(FetchContent)

message(STATUS "FetchContent: SDL2")

FetchContent_Declare(
  sdl2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-2.28.0 # Specify SDL2 version or commit hash
)

FetchContent_MakeAvailable(sdl2)

set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(SDL2_BUILD_SHARED OFF CACHE BOOL "" FORCE)

# Set global variables
set(SDL2_INCLUDE_DIR ${sdl2_SOURCE_DIR}/include CACHE INTERNAL "SDL2 include directory")
set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main CACHE INTERNAL "SDL2 libraries")
