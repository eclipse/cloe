cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_gndtruth_extractor LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
endif()

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME controller_gndtruth_extractor
    SOURCES
        src/gndtruth_extractor.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-gndtruth-extractor
        src/gndtruth_extractor_test.cpp
    )
    set_target_properties(test-gndtruth-extractor PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-gndtruth-extractor
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        cloe::runtime
        cloe::models
    )
    gtest_add_tests(TARGET test-gndtruth-extractor)
endif()
