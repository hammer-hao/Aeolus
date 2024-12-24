# Fetch nanoflann Library
include(FetchContent)

message(STATUS "FetchContent: nanoflann")

# Prevent install/uninstall rules
set(CMAKE_SKIP_INSTALL_RULES ON CACHE BOOL "Disable install rules")

FetchContent_Declare(
  nanoflann
  GIT_REPOSITORY https://github.com/jlblancoc/nanoflann.git
  GIT_TAG        v1.6.2
)

# Add this before calling FetchContent_MakeAvailable for nanoflann
set(NANOFLANN_BUILD_EXAMPLES OFF CACHE BOOL "Disable building nanoflann examples")
set(NANOFLANN_BUILD_TESTS OFF CACHE BOOL "Disable nanoflann tests")

FetchContent_MakeAvailable(nanoflann)

# Expose nanoflann's include directory for the parent scope
set(NANOFLANN_INCLUDE_DIR ${nanoflann_SOURCE_DIR}/include PARENT_SCOPE) # This makes it accessible in parent scope

