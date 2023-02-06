cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_virtue LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME controller_virtue
    SOURCES
        src/virtue.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
)
