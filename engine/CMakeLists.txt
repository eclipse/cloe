cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe_engine LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(fable REQUIRED QUIET)
    find_package(cloe-runtime REQUIRED QUIET)
    find_package(cloe-models REQUIRED QUIET)
endif()
find_package(Boost REQUIRED QUIET)
find_package(CLI11 REQUIRED QUIET)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED QUIET)

include(TargetLinting)
include(GNUInstallDirs)

# Compiler definitions
string(TIMESTAMP CLOE_ENGINE_TIMESTAMP "%Y-%m-%d")
set(CLOE_ENGINE_VERSION ${CLOE_PROJECT_VERSION})
set(PROJECT_GIT_REF "unknown")

# Library libstack ---------------------------------------------------
message(STATUS "Building cloe-stacklib library.")
add_library(cloe-stacklib STATIC
    src/config.hpp
    src/stack.hpp
    src/stack.cpp
    src/stack_factory.hpp
    src/stack_factory.cpp
    src/plugin.hpp
    src/plugin.cpp

    # Built-in plugins:
    src/plugins/nop_controller.cpp
    src/plugins/nop_controller.hpp
    src/plugins/nop_simulator.cpp
    src/plugins/nop_simulator.hpp
)
add_library(cloe::stacklib ALIAS cloe-stacklib)
set_target_properties(cloe-stacklib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME stack
)
target_include_directories(cloe-stacklib
  PRIVATE
    src
)
target_link_libraries(cloe-stacklib
  PUBLIC
    cloe::runtime
    cloe::models
    fable::fable
    Boost::headers
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)

    message(STATUS "Building test-stacklib executable.")
    add_executable(test-stacklib
        src/stack_test.cpp
        src/stack_component_test.cpp
    )
    set_target_properties(test-stacklib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-stacklib
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        cloe::models
        cloe::stacklib
    )
    gtest_add_tests(TARGET test-stacklib)
endif()

# Library libengine ----------------------------------------------
message(STATUS "Building cloe-enginelib library.")
add_library(cloe-enginelib STATIC
    src/coordinator.cpp
    src/coordinator.hpp
    src/registrar.hpp
    # These are added below and depend on CLOE_ENGINE_WITH_SERVER:
    # src/server.cpp
    # src/server_mock.cpp
    src/simulation.cpp
    src/simulation.hpp
    src/simulation_context.cpp
    src/simulation_context.hpp
    src/simulation_progress.hpp
    src/utility/command.cpp
    src/utility/command.hpp
    src/utility/defer.hpp
    src/utility/progress.hpp
    src/utility/state_machine.hpp
    src/utility/time_event.hpp
)
add_library(cloe::enginelib ALIAS cloe-enginelib)
set_target_properties(cloe-enginelib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME engine
)
target_compile_definitions(cloe-enginelib
  PUBLIC
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)
target_include_directories(cloe-enginelib
  PRIVATE
    src
)
target_link_libraries(cloe-enginelib
  PUBLIC
    cloe::stacklib
    cloe::models
    cloe::runtime
    fable::fable
    boost::boost
    Threads::Threads
)

option(CLOE_ENGINE_WITH_SERVER "Enable integrated server component?" ON)
if(CLOE_ENGINE_WITH_SERVER)
  if(CLOE_FIND_PACKAGES)
    find_package(cloe-oak REQUIRED QUIET)
  endif()
  target_sources(cloe-enginelib PRIVATE src/server.cpp)
  target_link_libraries(cloe-enginelib PRIVATE cloe::oak)
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_SERVER=1)
else()
  target_sources(cloe-enginelib PRIVATE src/server_mock.cpp)
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_SERVER=0)
endif()

# Executable ---------------------------------------------------------
message(STATUS "Building cloe-engine executable [with server=${CLOE_ENGINE_WITH_SERVER}].")
add_executable(cloe-engine
    src/main.cpp
    src/main_commands.hpp
    src/main_check.cpp
    src/main_dump.cpp
    src/main_run.cpp
    src/main_usage.cpp
    src/main_version.cpp
)
set_target_properties(cloe-engine PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME cloe-engine
)
target_compile_definitions(cloe-engine
  PRIVATE
    CLOE_ENGINE_VERSION="${CLOE_ENGINE_VERSION}"
    CLOE_ENGINE_TIMESTAMP="${CLOE_ENGINE_TIMESTAMP}"
)
target_include_directories(cloe-engine
  PRIVATE
    src
)
target_link_libraries(cloe-engine
  PRIVATE
    cloe::stacklib
    cloe::enginelib
    CLI11::CLI11
)

# Installation -------------------------------------------------------
install(TARGETS cloe-engine
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)
