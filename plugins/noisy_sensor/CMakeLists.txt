cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_noisy_sensors LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET component_noisy_object_sensor
    SOURCES
        src/noisy_object_sensor.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
)
cloe_add_plugin(
    TARGET component_noisy_lane_sensor
    SOURCES
        src/noisy_lane_sensor.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
)

option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-noisy-sensor
        src/noisy_sensor_test.cpp
    )
    set_target_properties(test-noisy-sensor PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-noisy-sensor
      PRIVATE
        CONAN_PKG::gtest
        CONAN_PKG::cloe-runtime
    )
    gtest_add_tests(TARGET test-noisy-sensor)
endif()
