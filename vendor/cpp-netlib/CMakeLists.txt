cmake_minimum_required(VERSION 3.7)

project(cpp-netlib
    LANGUAGES CXX
)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS)

conan_set_find_paths()
conan_set_find_library_paths()

add_subdirectory(cpp-netlib)
