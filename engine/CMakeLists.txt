cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_engine LANGUAGES CXX)

set(target cloe-engine)
set(output cloe-engine)
set(libstack cloe-stack)

option(BuildTests "Build tests?" ON)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(TargetLinting)

# Compiler definitions
string(TIMESTAMP CLOE_ENGINE_TIMESTAMP "%Y-%m-%d")
set(CLOE_ENGINE_VERSION ${CLOE_PROJECT_VERSION})
set(PROJECT_GIT_REF "unknown")

# Library ------------------------------------------------------------
add_library(${libstack}
    src/stack.cpp
    src/plugin.cpp
)
set_target_properties(${libstack} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(${libstack}
  PRIVATE
    src
)
target_link_libraries(${libstack}
  PUBLIC
    CONAN_PKG::cloe-runtime
    CONAN_PKG::fable
    pthread
    dl
)

if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    set(test-libstack test-${libstack})
    add_executable(${test-libstack}
        src/stack_test.cpp
    )
    set_target_properties(${test-libstack} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(${test-libstack}
        CONAN_PKG::gtest
        CONAN_PKG::boost
        CONAN_PKG::cloe-models
        ${libstack}
    )
    gtest_add_tests(TARGET ${test-libstack})
endif()

# Executable ---------------------------------------------------------
add_executable(${target}
    src/main.cpp
    src/main_stack.cpp
    src/coordinator.cpp
    src/plugins/nop_controller.cpp
    src/plugins/nop_simulator.cpp
    src/simulation.cpp
    src/simulation_context.cpp
    src/utility/command.cpp
)
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME ${output}
)
target_compile_definitions(${target}
    PRIVATE
        CLOE_ENGINE_VERSION="${CLOE_ENGINE_VERSION}"
        CLOE_ENGINE_TIMESTAMP="${CLOE_ENGINE_TIMESTAMP}"
)
set_target_linting(${target})
target_include_directories(${target}
  PRIVATE
    src
)
target_link_libraries(${target}
  PRIVATE
    CONAN_PKG::cli11
    CONAN_PKG::cloe-oak
    CONAN_PKG::cloe-models
    ${libstack}
)
target_compile_definitions(${target}
  PRIVATE
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${target}
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)
