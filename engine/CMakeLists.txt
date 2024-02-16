cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_engine LANGUAGES CXX)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(fable REQUIRED)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
    find_package(cloe-stacklib REQUIRED)
    find_package(cloe-simulation REQUIRED)
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

# Library libengine ----------------------------------------------
add_library(cloe-enginelib STATIC
    src/lua_setup.hpp
    src/lua_setup.cpp
    src/lua_setup_duration.cpp
    src/lua_setup_fs.cpp
    src/lua_setup_stack.cpp
    src/lua_setup_sync.cpp
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
    sol2::sol2
    fable::fable
    cloe::runtime
    cloe::stacklib
    cloe::simulation
)


#include(CTest)
#if(BUILD_TESTING)
#    find_package(GTest REQUIRED)
#    include(GoogleTest)
#    add_executable(test-enginelib
#        src/lua_stack_test.cpp
#    )
#    set_target_properties(test-enginelib PROPERTIES
#        CXX_STANDARD 17
#        CXX_STANDARD_REQUIRED ON
#    )
#    target_link_libraries(test-enginelib
#        GTest::gtest
#        GTest::gtest_main
#        Boost::boost
#        cloe::models
#        cloe::stacklib
#        cloe::simulation
#    )
#    gtest_add_tests(TARGET test-enginelib)
#endif()

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

option(CLOE_ENGINE_WITH_LRDB "Enable LRDB Lua Debugger?" ON)
if(CLOE_ENGINE_WITH_LRDB)
    message(STATUS "-> Enable LRDB component")
    add_subdirectory(vendor/lrdb)
    target_sources(cloe-engine PRIVATE src/lua_debugger.cpp)
    target_link_libraries(cloe-engine PRIVATE lrdb::lrdb)
    target_compile_definitions(cloe-engine PUBLIC CLOE_ENGINE_WITH_LRDB=1)
else()
    message(STATUS "-> Disable LRDB component")
    target_compile_definitions(cloe-engine PUBLIC CLOE_ENGINE_WITH_LRDB=0)
endif()

# Installation -------------------------------------------------------
install(TARGETS cloe-engine
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY lua/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe/lua
)
