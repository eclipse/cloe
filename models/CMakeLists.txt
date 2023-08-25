cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe-models LANGUAGES CXX)

include(GNUInstallDirs)
include(TargetLinting)

# Module -------------------------------------------------------------
set(target cloe-models)
set(alias cloe::models)

find_package(cloe-runtime REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost COMPONENTS headers REQUIRED)
find_package(sol2 REQUIRED)

message(STATUS "-> Building ${output} library.")
file(GLOB ${target}_PUBLIC_HEADERS "include/**/*.hpp")
add_library(${target}
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/cloe/component/lane_boundary.cpp
    src/cloe/component/utility/ego_sensor_canon.cpp
    src/cloe/component/utility/steering_utils.cpp
    src/cloe/utility/actuation_state.cpp

    # For IDE integration
    ${${target}_PUBLIC_HEADERS}
)
add_library(${alias} ALIAS ${target})
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${CLOE_PROJECT_VERSION}
)
set_target_linting(${target})
target_include_directories(${target}
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(${target}
  PUBLIC
    cloe::runtime
    Boost::headers
    Eigen3::Eigen
    sol2::sol2
)

# Testing -------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-models
        # find src -type f -name "*_test.cpp"
        src/cloe/component/gearbox_actuator_test.cpp
        src/cloe/component/latlong_actuator_test.cpp
        src/cloe/component/utility/steering_utils_test.cpp
        src/cloe/utility/actuation_level_test.cpp
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
        ${target}
    )
    gtest_add_tests(TARGET test-models)
endif()

# Installation -------------------------------------------------------
install(TARGETS ${target}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
