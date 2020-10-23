cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

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
