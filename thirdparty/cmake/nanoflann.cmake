# Fetch nanoflann Library
include(FetchContent)

message(STATUS "FetchContent: nanoflann")

FetchContent_Declare(
  nanoflann
  GIT_REPOSITORY https://github.com/jlblancoc/nanoflann.git
  GIT_TAG        v1.4.2
)

FetchContent_MakeAvailable(nanoflann)

# Expose nanoflann's include directory for the parent scope
set(NANOFLANN_INCLUDE_DIR ${nanoflann_SOURCE_DIR}/include PARENT_SCOPE) # This makes it accessible in parent scope
