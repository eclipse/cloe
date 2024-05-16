cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(fable_example_simple_config LANGUAGES CXX)

find_package(CLI11 REQUIRED QUIET)
find_package(fable REQUIRED QUIET)

# Executable ---------------------------------------------------------
add_executable(simple-config
    src/main.cpp
)
set_target_properties(simple-config PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(simple-config
  PRIVATE
    fable::fable
    CLI11::CLI11
)
