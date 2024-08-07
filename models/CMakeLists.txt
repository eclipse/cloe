cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe-models LANGUAGES CXX)

include(GNUInstallDirs)
include(TargetLinting)

# Module -------------------------------------------------------------
set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED QUIET)
endif()
find_package(Eigen3 REQUIRED QUIET)
find_package(Boost COMPONENTS headers REQUIRED QUIET)
find_package(sol2 REQUIRED QUIET)

message(STATUS "Building cloe-models library.")
file(GLOB cloe-models_PUBLIC_HEADERS "include/**/*.hpp")
add_library(cloe-models
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/cloe/component/lane_boundary.cpp
    src/cloe/component/utility/ego_sensor_canon.cpp
    src/cloe/component/utility/steering_utils.cpp
    src/cloe/utility/actuation_state.cpp
    src/cloe/utility/lua_types.cpp
    src/cloe/plugins/nop_controller.cpp
    src/cloe/plugins/nop_simulator.cpp

    # For IDE integration
    ${cloe-models_PUBLIC_HEADERS}
)
add_library(cloe::models ALIAS cloe-models)
set_target_properties(cloe-models PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${CLOE_PROJECT_VERSION}
)
target_include_directories(cloe-models
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(cloe-models
  PUBLIC
    cloe::runtime
    Boost::headers
    Eigen3::Eigen
    sol2::sol2
)

# Testing -------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)

    message(STATUS "Building test-models executable.")
    add_executable(test-models
        # find src -type f -name "*_test.cpp"
        src/cloe/component/gearbox_actuator_test.cpp
        src/cloe/component/latlong_actuator_test.cpp
        src/cloe/component/utility/steering_utils_test.cpp
        src/cloe/utility/actuation_level_test.cpp
        src/cloe/utility/frustum_culling_test.cpp
        src/cloe/utility/lua_types_test.cpp
    )
    set_target_properties(test-models PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-models
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        cloe::runtime
        cloe-models
    )
    gtest_add_tests(TARGET test-models)
endif()

# Installation -------------------------------------------------------
install(TARGETS cloe-models
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
