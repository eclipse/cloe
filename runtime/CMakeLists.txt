cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe-runtime LANGUAGES CXX)

# NOTE: The variable CLOE_VERSION is used throughout this CMakeLists file
# and is supplied from Conan or by hand on the command line.
set(CLOE_VERSION "0.0.0-undefined" CACHE STRING "Cloe version as MAJOR.MINOR.PATCH string")
set(CLOE_VERSION_U32 0 CACHE STRING "Cloe version as (MAJOR<<16)|(MINOR<<8)|PATCH integer")

include(GNUInstallDirs)

# Library -------------------------------------------------------------
set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(fable REQUIRED QUIET)
endif()
find_package(Boost COMPONENTS headers filesystem iostreams system REQUIRED QUIET)
find_package(spdlog REQUIRED QUIET)
find_package(inja REQUIRED QUIET)
if(NOT TARGET pantor::inja)
    add_library(pantor::inja ALIAS inja::inja)
endif()
find_package(incbin REQUIRED QUIET)
find_package(sol2 REQUIRED QUIET)

file(GLOB cloe-runtime_PUBLIC_HEADERS "include/**/*.hpp")
message(STATUS "Building cloe-runtime library.")
add_library(cloe-runtime SHARED
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
    src/cloe/utility/output_serializer_json.cpp
    src/cloe/utility/std_extensions.cpp
    src/cloe/utility/uid_tracker.cpp
    src/cloe/utility/xdg.cpp

    # For IDE integration
    ${cloe-runtime_PUBLIC_HEADERS}
)
add_library(cloe::runtime ALIAS cloe-runtime)
set_target_properties(cloe-runtime PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${CLOE_VERSION}
)
configure_file(src/cloe/version.hpp.in include/cloe/version.hpp @ONLY)
target_include_directories(cloe-runtime
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(cloe-runtime
  PUBLIC
    Boost::headers
    Boost::filesystem
    Boost::iostreams
    Boost::system
    fable::fable
    spdlog::spdlog
    sol2::sol2
  INTERFACE
    pantor::inja
    incbin::incbin
)
target_compile_definitions(cloe-runtime
  PRIVATE
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
  PUBLIC
    _USE_MATH_DEFINES=1
    SOL_ALL_SAFETIES_ON=1
)

# Testing -------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)

    message(STATUS "Building test-cloe executable.")
    add_executable(test-cloe
        # find src -type f -name "*_test.cpp"
        src/cloe/version_test.cpp
        src/cloe/utility/statistics_test.cpp
        src/cloe/utility/uid_tracker_test.cpp
    )
    set_target_properties(test-cloe PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-cloe
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        sol2::sol2
        cloe-runtime
    )
    gtest_add_tests(TARGET test-cloe)
endif()

# Installation --------------------------------------------------------
install(TARGETS cloe-runtime
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
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
