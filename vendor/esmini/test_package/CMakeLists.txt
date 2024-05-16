cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)
project(test_package LANGUAGES CXX)

find_package(esmini REQUIRED QUIET)

add_executable(${PROJECT_NAME} test_package.cpp)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        esmini::esmini
)
