cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe_plugin_clothoid_fit LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED QUIET)
    find_package(cloe-models REQUIRED QUIET)
endif()
find_package(Eigen3 REQUIRED QUIET)

add_library(g1-fitting-lib STATIC
    src/g1_fitting.cpp
)
set_target_properties(g1-fitting-lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET component_clothoid_fit
    SOURCES
        src/clothoid_fit.cpp
    LINK_LIBRARIES
        g1-fitting-lib
        Eigen3::Eigen
        cloe::runtime
        cloe::models
    COMPILE_DEFINITIONS
        PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)

    add_executable(test-clothoid-fit
        src/g1_fitting_test.cpp
        src/clothoid_test.cpp
    )
    set_target_properties(test-clothoid-fit PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-clothoid-fit
      PRIVATE
        g1-fitting-lib
        GTest::gtest
        GTest::gtest_main
        Eigen3::Eigen
        cloe::runtime
        cloe::models
    )
    gtest_add_tests(TARGET test-clothoid-fit)
endif()
