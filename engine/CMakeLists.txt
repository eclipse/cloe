cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(cloe_engine LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(fable REQUIRED QUIET)
    find_package(cloe-runtime REQUIRED QUIET)
    find_package(cloe-models REQUIRED QUIET)
    find_package(cloe-stack REQUIRED QUIET)
endif()
find_package(Boost REQUIRED QUIET)
find_package(CLI11 REQUIRED QUIET)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED QUIET)
find_package(sol2 REQUIRED QUIET)

include(TargetLinting)
include(GNUInstallDirs)

# Compiler definitions
string(TIMESTAMP CLOE_ENGINE_TIMESTAMP "%Y-%m-%d")
set(CLOE_ENGINE_VERSION ${CLOE_PROJECT_VERSION})
set(PROJECT_GIT_REF "unknown")

# Library libengine ----------------------------------------------
message(STATUS "Building cloe-enginelib library.")
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
    src/simulation_actions.hpp
    src/simulation_events.hpp
    src/simulation_outcome.hpp
    src/simulation_result.hpp
    src/simulation_probe.hpp
    src/simulation_statistics.hpp
    src/simulation_sync.hpp
    src/simulation_progress.hpp
    src/simulation_machine.hpp
    src/simulation_state_abort.cpp
    src/simulation_state_connect.cpp
    src/simulation_state_disconnect.cpp
    src/simulation_state_fail.cpp
    src/simulation_state_keep_alive.cpp
    src/simulation_state_pause.cpp
    src/simulation_state_probe.cpp
    src/simulation_state_reset.cpp
    src/simulation_state_resume.cpp
    src/simulation_state_start.cpp
    src/simulation_state_step_begin.cpp
    src/simulation_state_step_controllers.cpp
    src/simulation_state_step_end.cpp
    src/simulation_state_step_simulators.cpp
    src/simulation_state_stop.cpp
    src/simulation_state_success.cpp
    src/utility/command.cpp
    src/utility/command.hpp
    src/utility/defer.hpp
    src/utility/progress.hpp
    src/utility/state_machine.hpp
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
    cloe::stack
    cloe::models
    cloe::runtime
    fable::fable
    boost::boost
    sol2::sol2
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

option(CLOE_ENGINE_WITH_LRDB "Enable LRDB Lua Debugger?" ON)
if(CLOE_ENGINE_WITH_LRDB)
  add_subdirectory(vendor/lrdb)
  target_sources(cloe-enginelib PRIVATE src/lua_debugger.cpp)
  target_link_libraries(cloe-enginelib PRIVATE lrdb::lrdb)
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_LRDB=1)
else()
  target_compile_definitions(cloe-enginelib PUBLIC CLOE_ENGINE_WITH_LRDB=0)
endif()

include(CTest)
if(BUILD_TESTING)
    message(STATUS "Building test-enginelib executable.")
    find_package(GTest REQUIRED QUIET)
    include(GoogleTest)
    add_executable(test-enginelib
        src/lua_stack_test.cpp
        src/lua_setup_test.cpp
    )
    target_compile_definitions(test-enginelib
      PRIVATE
        CLOE_LUA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lua"
    )
    set_target_properties(test-enginelib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-enginelib
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        cloe::models
        cloe::stack
        cloe::enginelib
    )
    gtest_add_tests(TARGET test-enginelib)
endif()

# Executable ---------------------------------------------------------
message(STATUS "Building cloe-engine executable [with server=${CLOE_ENGINE_WITH_SERVER}, lrdb=${CLOE_ENGINE_WITH_LRDB}].")
add_subdirectory(vendor/linenoise)
add_executable(cloe-engine
    src/main.cpp
    src/main_commands.hpp
    src/main_commands.cpp
    src/main_check.cpp
    src/main_dump.cpp
    src/main_probe.cpp
    src/main_run.cpp
    src/main_shell.cpp
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
    cloe::stack
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
