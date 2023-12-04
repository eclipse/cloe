cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_mocks LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
endif()

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
