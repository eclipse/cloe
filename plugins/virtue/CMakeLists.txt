cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_virtue LANGUAGES CXX)

find_package(cloe-runtime REQUIRED)
find_package(cloe-models REQUIRED)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME controller_virtue
    SOURCES
        src/virtue.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)
