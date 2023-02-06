cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(cloe_plugin_basic LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET ${PROJECT_NAME}
    OUTPUT_NAME controller_basic
    SOURCES
        src/basic.cpp
    LINK_LIBRARIES
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
    COMPILE_DEFINITIONS
        PROJECT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)

option(BuildTests "Build tests?" ON)
if(BuildTests)
    message(STATUS "-> Enable testing")
    enable_testing()
    include(GoogleTest)

    add_executable(test-basic-controller
        src/hmi_contact_test.cpp
    )
    set_target_properties(test-basic-controller PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-basic-controller
      PRIVATE
        CONAN_PKG::gtest
        CONAN_PKG::cloe-runtime
    )
    gtest_add_tests(TARGET test-basic-controller)
endif()
