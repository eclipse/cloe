cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_engine LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(fable REQUIRED)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
endif()
find_package(Boost REQUIRED)
find_package(CLI11 REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(sol2 REQUIRED)

include(TargetLinting)
include(GNUInstallDirs)

# Compiler definitions
string(TIMESTAMP CLOE_ENGINE_TIMESTAMP "%Y-%m-%d")
set(CLOE_ENGINE_VERSION ${CLOE_PROJECT_VERSION})
set(PROJECT_GIT_REF "unknown")

# Library libstack ---------------------------------------------------
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
    find_package(GTest REQUIRED)
    include(GoogleTest)

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
add_library(cloe-enginelib STATIC
    src/lua_api.hpp
    src/lua_api.cpp
    src/lua_setup.hpp
    src/lua_setup.cpp
    src/lua_setup_duration.cpp
    src/lua_setup_fs.cpp
    src/lua_setup_stack.cpp
    src/lua_setup_sync.cpp
    src/coordinator.cpp
    src/coordinator.hpp
    src/lua_action.cpp
    src/lua_action.hpp
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
    SOL_ALL_SAFETIES_ON=1
    LRDB_USE_BOOST_ASIO=1
    CLOE_ENGINE_VERSION="${CLOE_ENGINE_VERSION}"
    CLOE_ENGINE_TIMESTAMP="${CLOE_ENGINE_TIMESTAMP}"
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
    sol2::sol2
    Threads::Threads
)

option(CLOE_ENGINE_WITH_SERVER "Enable integrated server component?" ON)
if(CLOE_ENGINE_WITH_SERVER)
  message(STATUS "-> Enable server component")
  if(CLOE_FIND_PACKAGES)
    find_package(cloe-oak REQUIRED)
  endif()
  target_sources(cloe-enginelib PRIVATE src/server.cpp)
  target_link_libraries(cloe-enginelib PRIVATE cloe::oak)
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_SERVER=1)
else()
  message(STATUS "-> Disable server component")
  target_sources(cloe-enginelib PRIVATE src/server_mock.cpp)
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_SERVER=0)
endif()

option(CLOE_ENGINE_WITH_LRDB "Enable LRDB Lua Debugger?" ON)
if(CLOE_ENGINE_WITH_LRDB)
  message(STATUS "-> Enable LRDB component")
  add_subdirectory(vendor/lrdb)
  target_sources(cloe-enginelib PRIVATE src/lua_debugger.cpp)
  target_link_libraries(cloe-enginelib PRIVATE lrdb::lrdb)
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_LRDB=1)
else()
  message(STATUS "-> Disable LRDB component")
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_LRDB=0)
endif()

if(BUILD_TESTING)
    add_executable(test-enginelib
        src/lua_stack_test.cpp
    )
    set_target_properties(test-enginelib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-enginelib
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        cloe::models
        cloe::stacklib
        cloe::enginelib
    )
    gtest_add_tests(TARGET test-enginelib)
endif()

# Executable ---------------------------------------------------------
add_subdirectory(vendor/linenoise)
add_executable(cloe-engine
    src/main.cpp
    src/main_commands.hpp
    src/main_check.cpp
    src/main_dump.cpp
    src/main_run.cpp
    src/main_usage.cpp
    src/main_shell.cpp
    src/main_version.cpp
)
set_target_properties(cloe-engine PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME cloe-engine
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
    linenoise::linenoise
)

# Installation -------------------------------------------------------
install(TARGETS cloe-engine
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY lua/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe/lua
)
