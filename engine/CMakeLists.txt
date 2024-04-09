cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_engine LANGUAGES CXX)

set(target cloe-engine)
set(output cloe-engine)
set(libstack cloe-stack)

find_package(cloe-runtime REQUIRED)
find_package(cloe-models REQUIRED)
find_package(fable REQUIRED)
find_package(Boost REQUIRED)
find_package(CLI11 REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(TargetLinting)
include(GNUInstallDirs)

# Compiler definitions
string(TIMESTAMP CLOE_ENGINE_TIMESTAMP "%Y-%m-%d")
set(CLOE_ENGINE_VERSION ${CLOE_PROJECT_VERSION})
set(PROJECT_GIT_REF "unknown")

# Library ------------------------------------------------------------
add_library(${libstack}
    src/stack.cpp
    src/plugin.cpp
)
set_target_properties(${libstack} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(${libstack}
  PRIVATE
    src
)
target_link_libraries(${libstack}
  PUBLIC
    cloe::runtime
    fable::fable
    Boost::headers
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    set(test-libstack test-${libstack})
    add_executable(${test-libstack}
        src/stack_test.cpp
    )
    set_target_properties(${test-libstack} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(${test-libstack}
        GTest::gtest
        GTest::gtest_main
        Boost::boost
        cloe::models
        ${libstack}
    )
    gtest_add_tests(TARGET ${test-libstack})
endif()

# Executable ---------------------------------------------------------
add_executable(${target}
    src/main.cpp
    src/main_stack.cpp
    src/coordinator.cpp
    src/plugins/nop_controller.cpp
    src/plugins/nop_simulator.cpp
    src/simulation.cpp
    src/simulation_context.cpp
    src/utility/command.cpp
)
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME ${output}
)
target_compile_definitions(${target}
  PRIVATE
    CLOE_ENGINE_VERSION="${CLOE_ENGINE_VERSION}"
    CLOE_ENGINE_TIMESTAMP="${CLOE_ENGINE_TIMESTAMP}"
    PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)
set_target_linting(${target})
target_include_directories(${target}
  PRIVATE
    src
)
target_link_libraries(${target}
  PRIVATE
    CLI11::CLI11
    cloe::models
    ${libstack}
)

option(CLOE_ENGINE_WITH_SERVER "Enable integrated server component?" ON)
if(CLOE_ENGINE_WITH_SERVER)
  message(STATUS "-> Enable server component")
  find_package(cloe-oak REQUIRED)
  target_sources(${target}
    PRIVATE
      src/server.cpp
  )
  target_link_libraries(${target}
    PRIVATE
      cloe::oak
  )
  target_compile_definitions(${target}
    PRIVATE
      CLOE_ENGINE_WITH_SERVER=1
  )
else()
  message(STATUS "-> Disable server component")
  target_sources(${target}
    PRIVATE
      src/server_mock.cpp
  )
  target_compile_definitions(${target}
    PRIVATE
      CLOE_ENGINE_WITH_SERVER=0
  )
endif()

# Installation -------------------------------------------------------
install(TARGETS ${target}
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)
