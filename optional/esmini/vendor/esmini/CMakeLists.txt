cmake_minimum_required(VERSION 3.8)

project(esmini_conan
    LANGUAGES CXX
)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS)

conan_set_find_paths()
conan_set_find_library_paths()

set(CMAKE_FIND_DEBUG_MODE TRUE)

add_subdirectory(${CMAKE_PROJECT_SUBDIR})
