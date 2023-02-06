cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe-models LANGUAGES CXX)

set(target cloe-models)
set(alias cloe::models)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(TargetLinting)

# Module -------------------------------------------------------------
message(STATUS "-> Building ${output} library.")
add_library(${target}
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/cloe/component/lane_boundary.cpp
    src/cloe/component/utility/ego_sensor_canon.cpp
    src/cloe/component/utility/steering_utils.cpp
    src/cloe/utility/actuation_state.cpp
)
add_library(${alias} ALIAS ${target})
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    VERSION ${CLOE_PROJECT_VERSION}
)
set_target_linting(${target})
target_include_directories(${target}
  PUBLIC
    include
)
target_link_libraries(${target}
  PUBLIC
    CONAN_PKG::cloe-runtime
    CONAN_PKG::boost
    CONAN_PKG::eigen
)

# Testing -------------------------------------------------------------
option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-models
        # find src -type f -name "*_test.cpp"
        src/cloe/component/gearbox_actuator_test.cpp
        src/cloe/component/latlong_actuator_test.cpp
        src/cloe/component/utility/steering_utils_test.cpp
        src/cloe/utility/actuation_level_test.cpp
    )
    set_target_properties(test-models PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-models
      PRIVATE
        CONAN_PKG::gtest
        CONAN_PKG::boost
        CONAN_PKG::cloe-runtime
        ${target}
    )
    gtest_add_tests(TARGET test-models)
endif()

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${target}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
