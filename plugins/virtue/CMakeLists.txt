cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_virtue LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
endif()

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
