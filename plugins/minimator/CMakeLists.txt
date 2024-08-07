cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe_plugin_minimator LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED QUIET)
    find_package(cloe-models REQUIRED QUIET)
endif()

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME simulator_minimator
    INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    SOURCES
        src/minimator.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)

    set(test-minimator test-${PROJECT_NAME})
    add_executable(${test-minimator}
        src/minimator_config_test.cpp
    )
    target_include_directories(${test-minimator}
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    set_target_properties(${test-minimator} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(${test-minimator}
        GTest::gtest
        GTest::gtest_main
        cloe::runtime
        cloe::models
    )
    gtest_add_tests(TARGET ${test-minimator})
endif()
