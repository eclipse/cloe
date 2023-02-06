cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_gndtruth_extractor LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME controller_gndtruth_extractor
    SOURCES
        src/gndtruth_extractor.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
)

include(CTest)
if(BUILD_TESTING)
    include(GoogleTest)

    add_executable(test-gndtruth-extractor
        src/gndtruth_extractor_test.cpp
    )
    set_target_properties(test-gndtruth-extractor PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-gndtruth-extractor
      PRIVATE
        CONAN_PKG::gtest
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
    )
    gtest_add_tests(TARGET test-gndtruth-extractor)
endif()
