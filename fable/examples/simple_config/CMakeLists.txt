cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(fable_example_simple_config LANGUAGES CXX)

find_package(CLI11 REQUIRED)
find_package(fable REQUIRED)

# Executable ---------------------------------------------------------
set(target simple-config)
add_executable(${target}
    src/main.cpp
)
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${target}
  PRIVATE
    fable::fable
    CLI11::CLI11
)
