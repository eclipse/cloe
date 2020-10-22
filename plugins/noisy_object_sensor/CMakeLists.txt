cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_noisy_object_sensor LANGUAGES CXX)

set(target component-noisy-object-sensor)
set(output component_noisy_object_sensor)
set(sources
    src/noisy.cpp
)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(TargetLinting)

# Module -------------------------------------------------------------
message(STATUS "-> Building ${output} module.")
add_library(${target} MODULE ${sources})
set(cloe_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib/cloe)
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    ARCHIVE_OUTPUT_DIRECTORY ${cloe_lib_dir}
    LIBRARY_OUTPUT_DIRECTORY ${cloe_lib_dir}
    RUNTIME_OUTPUT_DIRECTORY ${cloe_lib_dir}
    OUTPUT_NAME ${output}
    PREFIX ""
)
set_target_linting(${target})
target_compile_options(${target}
  PRIVATE
    -fvisibility=hidden -fvisibility-inlines-hidden
)
target_link_libraries(${target}
  PRIVATE
    CONAN_PKG::cloe-runtime
    CONAN_PKG::cloe-models
)
target_compile_definitions(${target}
  PRIVATE
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${target}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe
)
