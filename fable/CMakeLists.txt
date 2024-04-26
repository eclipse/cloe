cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(fable LANGUAGES CXX)

# NOTE: The variable FABLE_VERSION is used throughout this CMakeLists file
# and is supplied from Conan or by hand on the command line.
set(FABLE_VERSION "0.0.0-undefined" CACHE STRING "Fable version as MAJOR.MINOR.PATCH string")
set(FABLE_VERSION_U32 0 CACHE STRING "Fable version as (MAJOR<<16)|(MINOR<<8)|PATCH integer")

include(GNUInstallDirs)

# Library ------------------------------------------------------------
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

file(GLOB fable_PUBLIC_HEADERS "include/**/*.hpp")
add_library(fable GLOB)
add_library(fable::fable ALIAS fable)
set_target_properties(fable PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${FABLE_VERSION}
)
configure_file(src/fable/version.hpp.in include/fable/version.hpp @ONLY)
target_include_directories(fable
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(fable
  PUBLIC
    fmt::fmt
    nlohmann_json::nlohmann_json
    stdc++fs
)

option(FABLE_ALLOW_COMMENTS "Allow comments when parsing JSON?" ON)
if(FABLE_ALLOW_COMMENTS)
    target_compile_definitions(fable PRIVATE PARSE_JSON_WITH_COMMENTS=true)
else()
    target_compile_definitions(fable PRIVATE PARSE_JSON_WITH_COMMENTS=false)
endif()

# Testing ------------------------------------------------------------
set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    find_package(Boost COMPONENTS headers filesystem REQUIRED)
    find_package(sol2 REQUIRED)
    include(GoogleTest)

    add_executable(test-fable
        GLOB
    )
    set_target_properties(test-fable PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-fable
        GTest::gtest
        GTest::gtest_main
        Boost::headers
        Boost::filesystem
        sol2::sol2
        fable
    )
    target_compile_definitions(test-fable
      PUBLIC
        SOL_ALL_SAFETIES_ON=1
    )
    gtest_add_tests(TARGET test-fable)
endif()
