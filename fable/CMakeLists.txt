cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(fable LANGUAGES CXX)

set(target fable)
set(alias fable::fable)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Library ------------------------------------------------------------
add_library(${target}
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/fable/conf.cpp
    src/fable/json.cpp
    src/fable/schema.cpp
    src/fable/schema/struct.cpp
    src/fable/schema/string.cpp
    src/fable/schema/variant.cpp
    src/fable/schema/path.cpp
    src/fable/environment.cpp
    src/fable/utility.cpp
)
add_library(${alias} ALIAS ${target})
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    VERSION ${FABLE_VERSION}
)
target_include_directories(${target}
  PUBLIC
    include
)
target_link_libraries(${target}
  PUBLIC
    CONAN_PKG::boost
    CONAN_PKG::fmt
    CONAN_PKG::nlohmann_json
)

# Testing ------------------------------------------------------------
option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-fable
        # find src -type f -name "*_test.cpp"
        src/fable/environment_test.cpp
        src/fable/schema/const_test.cpp
        src/fable/schema/enum_test.cpp
        src/fable/schema/number_test.cpp
        src/fable/schema/optional_test.cpp
        src/fable/schema_test.cpp
    )
    set_target_properties(test-fable PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-fable
        CONAN_PKG::gtest
        CONAN_PKG::boost
        ${target}
    )
    gtest_add_tests(TARGET test-fable)
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
