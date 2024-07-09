cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe_stack LANGUAGES CXX)

include(GNUInstallDirs)
include(TargetLinting)

# Module -------------------------------------------------------------
set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED QUIET)
    find_package(cloe-models REQUIRED QUIET)
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED QUIET)
find_package(Boost COMPONENTS headers REQUIRED QUIET)

message(STATUS "Building cloe-stack library.")
file(GLOB cloe-stack_PUBLIC_HEADERS "include/**/*.hpp")
add_library(cloe-stack STATIC
    ${cloe-stack_PUBLIC_HEADERS}
    src/cloe/stack.cpp
    src/cloe/stack_factory.cpp
    src/cloe/plugin_loader.cpp
)
add_library(cloe::stack ALIAS cloe-stack)
set_target_properties(cloe-stack PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(cloe-stack
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(cloe-stack
  PUBLIC
    cloe::runtime
    cloe::models
    fable::fable
    Boost::headers
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# Testing -------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)
    message(STATUS "Building test-stack executable.")
    add_executable(test-stack
        src/cloe/stack_test.cpp
        src/cloe/stack_component_test.cpp
    )
    set_target_properties(test-stack PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-stack
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        cloe::models
        cloe::stack
    )
    gtest_add_tests(TARGET test-stack)
endif()

# Installation -------------------------------------------------------
install(TARGETS cloe-stack
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
