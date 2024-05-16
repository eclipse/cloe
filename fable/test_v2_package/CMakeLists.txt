cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)

project(fable_examples)

set(examples_prefix ".")
if (AS_TEST_PACKAGE)
    set(examples_prefix "../examples")
endif()

add_subdirectory(${examples_prefix}/contacts contacts)
add_subdirectory(${examples_prefix}/simple_config simple_config)
