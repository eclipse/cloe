cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_basic LANGUAGES CXX)

find_package(pybind11 CONFIG REQUIRED)
set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
    find_package(cloe-models REQUIRED)
    find_package(cloe-databroker-bindings REQUIRED)
endif()

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME controller_basic
    SOURCES
        src/basic.cpp
    LINK_LIBRARIES
        cloe::runtime
        cloe::models
    COMPILE_DEFINITIONS
        PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

pybind11_add_module(_basic_bindings
        src/basic_bindings.cpp
)
target_include_directories(_basic_bindings PRIVATE src/)
target_link_libraries(_basic_bindings PUBLIC cloe::runtime cloe::models cloe::databroker-bindings)
set_target_properties(_basic_bindings PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

set_target_properties(_basic_bindings
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cloe/python"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/cloe/python"
    )


install(TARGETS _basic_bindings LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe/python)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-basic-controller
        src/hmi_contact_test.cpp
    )
    set_target_properties(test-basic-controller PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-basic-controller
      PRIVATE
        GTest::gtest
        GTest::gtest_main
        cloe::runtime
    )
    gtest_add_tests(TARGET test-basic-controller)
endif()
