# This module provides some useful functions for defining Cloe plugins.
#

include(GNUInstallDirs)
include(TargetLinting)

function(cloe_add_plugin)
    set(options
        LINT
    )
    set(one_value_args
        TARGET              # [required]
        OUTPUT_NAME         # [default=${TARGET}]
        OUTPUT_DIRECTORY    # [default=${CMAKE_CURRENT_BINARY_DIR}/lib/cloe]
        CXX_STANDARD        # [default=14]
        PYTHON_DRIVER
    )
    set(multi_value_args
        SOURCES
        COMPILE_OPTIONS
        COMPILE_DEFINITIONS
        INCLUDE_DIRECTORIES
        LINK_LIBRARIES
    )
    cmake_parse_arguments(_ARG
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    # Set default arguments:
    if(NOT DEFINED _ARG_TARGET)
        message(SEND_ERROR "cloe_add_plugin requires property TARGET to be set")
    endif()
    set(target ${_ARG_TARGET})
    if(NOT DEFINED _ARG_OUTPUT_NAME)
        set(_ARG_OUTPUT_NAME ${target})
    endif()
    if(NOT DEFINED _ARG_CXX_STANDARD)
        set(_ARG_CXX_STANDARD 17)
    endif()
    if(NOT DEFINED _ARG_SOURCES)
        message(SEND_ERROR "cloe_add_plugin requires property SOURCES to be set")
    endif()
    if(NOT DEFINED _ARG_OUTPUT_DIRECTORY)
        set(_ARG_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/cloe)
    endif()

    # Add the cloe plugin target:
    message(STATUS "-> Building ${_ARG_OUTPUT_NAME} plugin.")
    add_library(${target} MODULE
        ${_ARG_SOURCES}
    )
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD ${_ARG_CXX_STANDARD}
        CXX_STANDARD_REQUIRED ON

        LIBRARY_OUTPUT_DIRECTORY ${_ARG_OUTPUT_DIRECTORY}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${_ARG_OUTPUT_DIRECTORY}
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${_ARG_OUTPUT_DIRECTORY}
        LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${_ARG_OUTPUT_DIRECTORY}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${_ARG_OUTPUT_DIRECTORY}

        RUNTIME_OUTPUT_DIRECTORY ${_ARG_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${_ARG_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${_ARG_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${_ARG_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${_ARG_OUTPUT_DIRECTORY}

        OUTPUT_NAME ${_ARG_OUTPUT_NAME}
        PREFIX ""
    )
    if(${_ARG_LINT})
        set_target_linting(${target})
    endif()
    target_compile_options(${target}
      PRIVATE
        -fvisibility=hidden -fvisibility-inlines-hidden
        ${_ARG_COMPILE_OPTIONS}
    )
    if(DEFINED _ARG_COMPILE_DEFINITIONS)
        target_compile_definitions(${target}
          PRIVATE
            ${_ARG_COMPILE_DEFINITIONS}
        )
    endif()
    if(DEFINED _ARG_INCLUDE_DIRECTORIES)
        target_include_directories(${target}
          PRIVATE
            ${_ARG_INCLUDE_DIRECTORIES}
        )
    endif()
    if(DEFINED _ARG_LINK_LIBRARIES)
        target_link_libraries(${target}
          PRIVATE
            ${_ARG_LINK_LIBRARIES}
        )
    endif()

    if(DEFINED _ARG_PYTHON_DRIVER)
        set(python_driver_output ${_ARG_OUTPUT_DIRECTORY}/${_ARG_OUTPUT_NAME}.py)
        add_custom_target(${target}-driver ALL DEPENDS ${python_driver_output})
        add_custom_command(
            OUTPUT ${python_driver_output}
            DEPENDS ${_ARG_PYTHON_DRIVER}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${_ARG_OUTPUT_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E copy ${_ARG_PYTHON_DRIVER} ${python_driver_output}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
        install(FILES
            ${python_driver_output}
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe
        )
    endif()

    install(TARGETS ${target}
        LIBRARY
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe
    )
endfunction()
