cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe-osi LANGUAGES CXX)

include(GNUInstallDirs)
include(TargetLinting)

# Module -------------------------------------------------------------

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-models REQUIRED QUIET)
    find_package(cloe-runtime REQUIRED QUIET)
endif()
find_package(Eigen3 REQUIRED QUIET)
find_package(Boost COMPONENTS headers REQUIRED QUIET)
find_package(open_simulation_interface REQUIRED QUIET)

message(STATUS "Building cloe-osi library.")
file(GLOB cloe-osi_PUBLIC_HEADERS "include/**/*.hpp")
add_library(cloe-osi
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/cloe/component/osi_sensor.cpp
    src/cloe/utility/osi_ground_truth.cpp
    src/cloe/utility/osi_message_handler.cpp
    src/cloe/utility/osi_transceiver_tcp.cpp
    src/cloe/utility/osi_utils.cpp

    # For IDE integration
    ${cloe-osi_PUBLIC_HEADERS}
)
add_library(cloe::osi ALIAS cloe-osi)
set_target_properties(cloe-osi PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${CLOE_PROJECT_VERSION}
)
target_include_directories(cloe-osi
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(cloe-osi
  PUBLIC
    cloe::models
    cloe::runtime
    Boost::headers
    Eigen3::Eigen
    open_simulation_interface::open_simulation_interface
)

# Testing -------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)

    message(STATUS "Building test-osi executable.")
    add_executable(test-osi
        # find src -type f -name "*_test.cpp"
        src/cloe/component/osi_sensor_test.cpp
        src/cloe/utility/osi_test.cpp
    )
    set_target_properties(test-osi PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-osi
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        cloe::runtime
        cloe::models
        cloe::osi
        open_simulation_interface::open_simulation_interface
    )
    gtest_add_tests(TARGET test-osi)
endif()

# Installation -------------------------------------------------------
install(TARGETS cloe-osi
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
