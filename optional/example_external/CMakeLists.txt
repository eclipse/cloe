cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe_plugin_example_external LANGUAGES CXX)

find_package(cloe-runtime REQUIRED QUIET)
find_package(cloe-models REQUIRED QUIET)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET controller_example_external
    SOURCES
        src/external_printer.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)
