# Fetch SDL2 Library
include(FetchContent)

message(STATUS "FetchContent: SDL2")

FetchContent_Declare(
  sdl2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-2.28.0 # Specify SDL2 version or commit hash
)

FetchContent_MakeAvailable(sdl2)

# Expose SDL2 include directory and libraries for the parent scope
set(SDL2_INCLUDE_DIR ${sdl2_SOURCE_DIR}/include PARENT_SCOPE)
set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main PARENT_SCOPE)
