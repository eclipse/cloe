cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(fable_example_contacts LANGUAGES CXX)

find_package(CLI11 REQUIRED QUIET)
find_package(fable REQUIRED QUIET)
find_package(fmt REQUIRED QUIET)

# Executable ---------------------------------------------------------
add_executable(contacts
    src/main.cpp
)
set_target_properties(contacts PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(contacts
  PRIVATE
    fable::fable
    fmt::fmt
    CLI11::CLI11
)
