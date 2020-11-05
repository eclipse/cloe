cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_osi_omni_sensor LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CloePluginSetup)
add_library(osi-object-lib STATIC
    src/osi_ground_truth.cpp
    src/osi_omni_sensor.cpp
    src/osi_transceiver_tcp.cpp
    src/osi_utils.cpp
)
target_include_directories(osi-object-lib
  PUBLIC
    include
)
target_link_libraries(osi-object-lib
  PUBLIC
    CONAN_PKG::cloe-runtime
    CONAN_PKG::cloe-models
    CONAN_PKG::open-simulation-interface
)
set_target_properties(osi-object-lib PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)
# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS osi-object-lib
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cloe
)

option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-osi-omni-sensor
        src/osi_test.cpp
    )
    set_target_properties(test-osi-omni-sensor PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-osi-omni-sensor
      PRIVATE
        CONAN_PKG::gtest
        osi-object-lib
    )
    gtest_add_tests(TARGET test-osi-omni-sensor)
endif()
