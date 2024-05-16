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
    src/stack.hpp
    src/stack.cpp
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

# Executable ---------------------------------------------------------
add_executable(cloe-engine
    src/main.cpp
    src/main_stack.cpp
    src/coordinator.cpp
    src/simulation.cpp
    src/simulation_context.cpp
    src/utility/command.cpp
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
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)
target_include_directories(cloe-engine
  PRIVATE
    src
)
target_link_libraries(cloe-engine
  PRIVATE
    CLI11::CLI11
    cloe::models
    cloe::stacklib
)

option(CLOE_ENGINE_WITH_SERVER "Enable integrated server component?" ON)
if(CLOE_ENGINE_WITH_SERVER)
  message(STATUS "-> Enable server component")
  if(CLOE_FIND_PACKAGES)
    find_package(cloe-oak REQUIRED QUIET)
  endif()
  target_sources(cloe-engine
    PRIVATE
      src/server.cpp
  )
  target_link_libraries(cloe-engine
    PRIVATE
      cloe::oak
  )
  target_compile_definitions(cloe-engine
    PRIVATE
      CLOE_ENGINE_WITH_SERVER=1
  )
else()
  message(STATUS "-> Disable server component")
  target_sources(cloe-engine
    PRIVATE
      src/server_mock.cpp
  )
  target_compile_definitions(cloe-engine
    PRIVATE
      CLOE_ENGINE_WITH_SERVER=0
  )
endif()

# Installation -------------------------------------------------------
install(TARGETS cloe-engine
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)
