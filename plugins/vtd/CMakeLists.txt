cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_vtd LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Cloe Plugin --------------------------------------------------------
include(CloePluginSetup)

add_library(vtd-object-lib STATIC
    src/omni_sensor_component.cpp
    src/osi_ground_truth.cpp
    src/osi_omni_sensor.cpp
    src/osi_sensor_component.cpp
    src/osi_transceiver_tcp.cpp
    src/osi_utils.cpp
    src/rdb_codec.cpp
    src/rdb_transceiver_shm.cpp
    src/rdb_transceiver_tcp.cpp
    src/scp_messages.cpp
    src/scp_transceiver.cpp
)
target_link_libraries(vtd-object-lib
  PUBLIC
    CONAN_PKG::vtd-api
    CONAN_PKG::cloe-runtime
    CONAN_PKG::cloe-models
    CONAN_PKG::boost
    CONAN_PKG::open-simulation-interface
)
set_target_properties(vtd-object-lib PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME simulator_vtd
    SOURCES
        src/vtd_binding.cpp
    LINK_LIBRARIES
        vtd-object-lib
    PYTHON_DRIVER module.py
)

option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-vtd-binding
        src/rdb_transceiver_tcp_test.cpp
        src/osi_test.cpp
        src/vtd_osi_test.cpp
    )
    set_target_properties(test-vtd-binding PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-vtd-binding
      PRIVATE
        CONAN_PKG::gtest
        vtd-object-lib
    )
    gtest_add_tests(TARGET test-vtd-binding)
endif()
