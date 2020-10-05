cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe-oak LANGUAGES CXX)

set(target cloe-oak)
set(alias cloe::oak)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(TargetLinting)

# Library ------------------------------------------------------------
add_library(${target}
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/oak/registrar.cpp
    src/oak/server.cpp
)
add_library(${alias} ALIAS ${target})
set_target_linting(${target})
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(${target}
  PUBLIC
    include
)
target_link_libraries(${target}
  PUBLIC
    CONAN_PKG::cloe-runtime
    CONAN_PKG::cpp-netlib
)

# Testing ------------------------------------------------------------
option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-oak
        # find src -type f -name "*_test.cpp"
        src/oak/route_muxer_test.cpp
    )
    target_link_libraries(test-oak
        CONAN_PKG::gtest
        CONAN_PKG::boost
        ${target}
    )
    gtest_add_tests(TARGET test-oak)
endif()

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${target}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
