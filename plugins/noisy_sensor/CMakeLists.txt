cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_noisy_sensor LANGUAGES CXX)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(TargetLinting)
include(GNUInstallDirs)

function(add_cloe_plugin target_output sources)
    message(STATUS "-> Building ${target_output} module.")
    add_library(${target_output} MODULE ${sources})
    set(cloe_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib/cloe)
    set_target_properties(${target_output} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        ARCHIVE_OUTPUT_DIRECTORY ${cloe_lib_dir}
        LIBRARY_OUTPUT_DIRECTORY ${cloe_lib_dir}
        RUNTIME_OUTPUT_DIRECTORY ${cloe_lib_dir}
        OUTPUT_NAME ${target_output}
        PREFIX ""
    )
    set_target_linting(${target_output})
    target_compile_options(${target_output}
      PRIVATE
        -fvisibility=hidden -fvisibility-inlines-hidden
    )
    target_link_libraries(${target_output}
      PRIVATE
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
    )
    target_compile_definitions(${target_output}
      PRIVATE
        PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
    )

    install(TARGETS ${target_output}
        LIBRARY
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe
    )
endfunction()

add_cloe_plugin(component_noisy_object_sensor src/noisy_object_sensor.cpp)
add_cloe_plugin(component_noisy_lane_sensor src/noisy_lane_sensor.cpp)
