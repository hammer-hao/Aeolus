# Fetch Eigen Library
include(FetchContent)

message(STATUS "FetchContent: Eigen")

FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG        3.4.0
)

# Disable unnecessary features
set(EIGEN_TESTS OFF CACHE BOOL "Disable Eigen unit tests")
set(EIGEN_BUILD_DOC OFF CACHE BOOL "Disable Eigen documentation build")
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "Disable pkgconfig generation for Eigen")

set (BUILD_TESTING OFF)

FetchContent_MakeAvailable(eigen)

# Expose Eigen's include directory for the parent scope
set(EIGEN_INCLUDE_DIR ${eigen_SOURCE_DIR} PARENT_SCOPE)
