cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(fable_stress_test LANGUAGES CXX)

set(LARGE_STRUCT_SIZE 1000 CACHE NUMBER "Number of members of Large struct")

find_package(CLI11 REQUIRED)
find_package(fable REQUIRED)
find_package(fmt REQUIRED)

# Executable ---------------------------------------------------------
add_custom_command(
    OUTPUT large_struct.hxx
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/gen.py ${LARGE_STRUCT_SIZE} ${CMAKE_CURRENT_BINARY_DIR}/large_struct.hxx
    VERBATIM
)
add_executable(stress
    src/main.cpp
    large_struct.hxx
)
target_include_directories(stress
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)
set_target_properties(stress PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(stress
  PRIVATE
    fable::fable
    fmt::fmt
    CLI11::CLI11
)
