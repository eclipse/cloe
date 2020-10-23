cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_mocks LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET controller_demo_stuck
    SOURCES
        src/demo_stuck.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
)
cloe_add_plugin(
    TARGET controller_demo_printer
    SOURCES
        src/demo_printer.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
)
