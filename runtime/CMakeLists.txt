cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe-runtime LANGUAGES CXX)

set(target cloe-runtime)
set(alias cloe::runtime)

# Conan / Editor -----------------------------------------------------
include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(cmake/TargetLinting.cmake)

# Library -------------------------------------------------------------
add_library(${target} SHARED
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/cloe/core/error.cpp
    src/cloe/core/duration.cpp
    src/cloe/core/logger.cpp
    src/cloe/component.cpp
    src/cloe/entity.cpp
    src/cloe/handler.cpp
    src/cloe/model.cpp
    src/cloe/simulator.cpp
    src/cloe/trigger.cpp
    src/cloe/trigger/evaluate_event.cpp
    src/cloe/trigger/example_actions.cpp
    src/cloe/vehicle.cpp
    src/cloe/utility/command.cpp
    src/cloe/utility/evaluate.cpp
    src/cloe/utility/output_serializer.cpp
    src/cloe/utility/std_extensions.cpp
    src/cloe/utility/uid_tracker.cpp
    src/cloe/utility/xdg.cpp
)
add_library(${alias} ALIAS ${target})
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    VERSION ${CLOE_PROJECT_VERSION}
)
set_target_linting(${target})
target_include_directories(${target}
  PUBLIC
    include
)
target_link_libraries(${target}
  PUBLIC
    CONAN_PKG::fable
    CONAN_PKG::boost
    CONAN_PKG::spdlog
)
target_compile_definitions(${target}
  PRIVATE
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
  PUBLIC
    _USE_MATH_DEFINES=1
)

# Testing -------------------------------------------------------------
option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-cloe
        # find src -type f -name "*_test.cpp"
        src/cloe/utility/statistics_test.cpp
        src/cloe/utility/uid_tracker_test.cpp
    )
    set_target_properties(test-cloe PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-cloe
      PRIVATE
        CONAN_PKG::gtest
        CONAN_PKG::boost
        ${target}
    )
    gtest_add_tests(TARGET test-cloe)
endif()

# Installation --------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${target}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY cmake/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cloe
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
