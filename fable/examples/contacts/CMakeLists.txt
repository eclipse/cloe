cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(fable_example_contacts LANGUAGES CXX)

find_package(CLI11 REQUIRED)
find_package(fable REQUIRED)
find_package(fmt REQUIRED)

# Executable ---------------------------------------------------------
set(target contacts)
add_executable(${target}
    src/main.cpp
)
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${target}
  PRIVATE
    fable::fable
    fmt::fmt
    CLI11::CLI11
)
