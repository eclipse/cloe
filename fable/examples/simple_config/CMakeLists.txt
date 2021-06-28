cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(fable_example_simple_config LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Executable ---------------------------------------------------------
set(target simple-config)
add_executable(${target}
    src/main.cpp
)
set_target_properties(${target} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${target}
  PRIVATE
    CONAN_PKG::fable
    CONAN_PKG::cli11
)
