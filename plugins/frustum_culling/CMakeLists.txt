cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_frustum_culling LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
endif()

include(CloePluginSetup)
cloe_add_plugin(
    TARGET component_frustum_culling_objects
    SOURCES
        src/frustum_culling_objects.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)
cloe_add_plugin(
    TARGET component_frustum_culling_lanes
    SOURCES
        src/frustum_culling_lanes.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)

set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-frustum-culling
        src/frustum_culling_test.cpp
    )
    set_target_properties(test-frustum-culling PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-frustum-culling
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        cloe::runtime
        cloe::models
    )
    gtest_add_tests(TARGET test-frustum-culling)
endif()
