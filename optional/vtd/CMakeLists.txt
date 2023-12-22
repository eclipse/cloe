cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_vtd LANGUAGES CXX)

# Cloe Plugin --------------------------------------------------------
find_package(vtd-api REQUIRED)
find_package(cloe-runtime REQUIRED)
find_package(cloe-models REQUIRED)
find_package(Boost REQUIRED)
find_package(cloe-osi REQUIRED)

add_library(vtd-object-lib STATIC
    src/omni_sensor_component.cpp
    src/osi_sensor_component.cpp
    src/rdb_codec.cpp
    src/rdb_transceiver_shm.cpp
    src/rdb_transceiver_tcp.cpp
    src/scp_messages.cpp
    src/scp_transceiver.cpp
)
target_link_libraries(vtd-object-lib
  PUBLIC
    vtd::api
    cloe::runtime
    cloe::models
    cloe::osi
    Boost::boost
)
set_target_properties(vtd-object-lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME simulator_vtd
    SOURCES
        src/scp_action.cpp
        src/vtd_binding.cpp
    LINK_LIBRARIES
        vtd-object-lib
    PYTHON_DRIVER module.py
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-vtd-binding
        src/rdb_transceiver_tcp_test.cpp
        src/vtd_osi_test.cpp
        src/vtd_data_conversion_test.cpp
    )
    set_target_properties(test-vtd-binding PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-vtd-binding
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        vtd-object-lib
    )
    gtest_add_tests(TARGET test-vtd-binding)
endif()
