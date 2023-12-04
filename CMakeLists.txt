# This CMakeLists.txt configures a super-build containing everything
# from this repo.
#
# It is currently experimental.
#

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe LANGUAGES CXX)

set(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/runtime/cmake")

# Since a super-build does not export packages individually via Conan,
# we cannot depend on Conan-generated CMake config files, instead we
# use the CMake targets directly as if they were already found.
set(CLOE_FIND_PACKAGES OFF CACHE BOOL "Call find_package() for cloe packages" FORCE)

# Ensure output goes to one place so cloe-launch can find the plugins
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Ensure we can test from this level.
include(CTest)

add_subdirectory(fable)
add_subdirectory(runtime)
add_subdirectory(models)
add_subdirectory(oak)
add_subdirectory(engine)
add_subdirectory(plugins)
