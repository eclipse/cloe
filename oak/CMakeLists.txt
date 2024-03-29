cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe-oak LANGUAGES CXX)

include(GNUInstallDirs)
include(TargetLinting)

# Library ------------------------------------------------------------
set(target cloe-oak)
set(alias cloe::oak)

find_package(cloe-runtime REQUIRED)
find_package(oatpp REQUIRED)

file(GLOB ${target}_PUBLIC_HEADERS "include/**/*.hpp")
add_library(${target}
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/oak/registrar.cpp
    src/oak/server.cpp

    # For IDE integration
    ${${target}_PUBLIC_HEADERS}
    src/oak/request_stub.hpp
    src/oak/curl.hpp
)
add_library(${alias} ALIAS ${target})
set_target_linting(${target})
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(${target}
  PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(${target}
  PUBLIC
    cloe::runtime
    oatpp::oatpp
    stdc++fs
)

# Testing ------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-oak
        # find src -type f -name "*_test.cpp"
        src/oak/route_muxer_test.cpp
        src/oak/server_test.cpp
    )
    set_target_properties(test-oak PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_include_directories(test-oak
      PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    )
    target_link_libraries(test-oak
        GTest::gtest
        GTest::gtest_main
        ${target}
    )
    gtest_add_tests(TARGET test-oak)
endif()

# Installation -------------------------------------------------------
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
