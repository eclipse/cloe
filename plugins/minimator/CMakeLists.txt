cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_minimator LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
endif()

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME simulator_minimator
    SOURCES
        src/minimator.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
)
