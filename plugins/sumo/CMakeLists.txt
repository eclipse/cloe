cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_sumo LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Cloe Plugin --------------------------------------------------------
include(CloePluginSetup)

cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME simulator_sumo
    SOURCES
        src/sumo.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
        CONAN_PKG::sumo
	PYTHON_DRIVER module.py
)
