cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_noisy_sensors LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
endif()

include(CloePluginSetup)
cloe_add_plugin(
    TARGET component_noisy_object_sensor
    SOURCES
        src/noisy_object_sensor.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)
cloe_add_plugin(
    TARGET component_noisy_lane_sensor
    SOURCES
        src/noisy_lane_sensor.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-noisy-sensor
        src/noisy_sensor_test.cpp
    )
    set_target_properties(test-noisy-sensor PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-noisy-sensor
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        cloe::runtime
    )
    gtest_add_tests(TARGET test-noisy-sensor)
endif()
