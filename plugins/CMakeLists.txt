# This CMakeLists.txt configures a super-build of all plugins.
#
# It is currently experimental.
#
# This supports two use-cases:
#
# 1. Creating a cloe-plugins-core package containing all plugins.
# 2. Creating a cloe package containing everything from this repo.
#
# Other use-cases are currently not supported.
#

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

if(NOT DEFINED PROJECT_NAME)
    project(cloe_plugins_core LANGUAGES CXX)

    # Speed up configuration by only finding Cloe packages once
    set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
    if(CLOE_FIND_PACKAGES)
        find_package(cloe-runtime REQUIRED)
        find_package(cloe-models REQUIRED)
        set(CLOE_FIND_PACKAGES OFF FORCE)
    endif()

    # Ensure output goes to one place so cloe-launch can find the plugins
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
elseif(NOT ${PROJECT_NAME} STREQUAL "cloe")
    message(FATAL_ERROR "Building super-project unequal to cloe not suppoorted.")
endif()

# Ensure we can test from this level.
include(CTest)

add_subdirectory(basic)
add_subdirectory(gndtruth_extractor)
add_subdirectory(minimator)
add_subdirectory(mocks)
add_subdirectory(noisy_sensor)
add_subdirectory(speedometer)
add_subdirectory(virtue)
