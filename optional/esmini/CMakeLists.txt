cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_esmini LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
    find_package(cloe-osi REQUIRED)
endif()
find_package(esmini REQUIRED)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME simulator_esmini
    SOURCES
        src/esmini.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
        cloe::osi
        esmini::esmini
    COMPILE_DEFINITIONS
        PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-esmini
        src/esmini_test.cpp
    )
    set_target_properties(test-esmini PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-esmini
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        cloe::runtime
        cloe::models
        cloe::osi
    )
    gtest_add_tests(TARGET test-esmini)
endif()
