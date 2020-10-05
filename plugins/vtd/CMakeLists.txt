cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_vtd LANGUAGES CXX)

set(target simulator-vtd)
set(output simulator_vtd)
set(config module.py)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(VTD)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(TargetLinting)

# Module -------------------------------------------------------------
message(STATUS "-> Building simulator_vtd.so plugin.")
add_library(${target} MODULE "")
target_sources(${target}
  PRIVATE
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
    src/vtd_binding.cpp

    ${VTD_RDBHANDLER_SOURCES}
)
set(cloe_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib/cloe)
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    ARCHIVE_OUTPUT_DIRECTORY ${cloe_lib_dir}
    LIBRARY_OUTPUT_DIRECTORY ${cloe_lib_dir}
    RUNTIME_OUTPUT_DIRECTORY ${cloe_lib_dir}
    OUTPUT_NAME ${output}
    PREFIX ""
)
set_target_linting(${target})
target_compile_options(${target}
  PRIVATE
    -fvisibility=hidden -fvisibility-inlines-hidden
)
target_compile_definitions(${target}
  PRIVATE
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)
target_include_directories(${target} SYSTEM
  PRIVATE
    ${VTD_INCLUDE_DIRS}
    ${VTD_RDBHANDLER_INCLUDE_DIRS}
)
target_link_libraries(${target}
  PRIVATE
    CONAN_PKG::cloe-runtime
    CONAN_PKG::cloe-models
    CONAN_PKG::boost
    CONAN_PKG::open-simulation-interface
)
set(config_output ${cloe_lib_dir}/${output}.py)
add_custom_target(${target}-config ALL DEPENDS ${config_output})
add_custom_command(
    OUTPUT ${config_output}
    DEPENDS ${config}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${cloe_lib_dir}
    COMMAND ${CMAKE_COMMAND} -E copy ${config} ${config_output}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

# Testing ------------------------------------------------------------
option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-vtd-binding
        src/rdb_transceiver_tcp.cpp
        src/rdb_transceiver_tcp_test.cpp
        src/osi_transceiver_tcp.cpp
        src/osi_utils.cpp
        src/osi_omni_sensor.cpp
        src/osi_ground_truth.cpp
        src/osi_sensor_component.cpp
        src/osi_test.cpp
        src/vtd_osi_test.cpp
    )
    target_include_directories(test-vtd-binding
      PRIVATE
        ${VTD_INCLUDE_DIRS}
        ${VTD_RDBHANDLER_INCLUDE_DIRS}
    )
    target_link_libraries(test-vtd-binding
      PRIVATE
        CONAN_PKG::gtest
        CONAN_PKG::boost
        CONAN_PKG::open-simulation-interface
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
    )
    gtest_add_tests(TARGET test-vtd-binding)
endif()

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${target}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe
)
install(FILES
    ${config_output}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe
)
