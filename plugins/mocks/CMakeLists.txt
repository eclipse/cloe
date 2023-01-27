cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_mocks LANGUAGES CXX)

find_package(cloe-runtime REQUIRED)
find_package(cloe-models REQUIRED)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET controller_demo_stuck
    SOURCES
        src/demo_stuck.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)
cloe_add_plugin(
    TARGET controller_demo_printer
    SOURCES
        src/demo_printer.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)
