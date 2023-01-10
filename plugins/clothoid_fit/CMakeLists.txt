cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_plugin_clothoid_fit LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_library(g1-fitting-lib STATIC
    src/g1_fitting.cpp
)
set_target_properties(g1-fitting-lib PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

include(CloePluginSetup)
cloe_add_plugin(
    TARGET component_clothoid_fit
    SOURCES
        src/clothoid_fit.cpp
    LINK_LIBRARIES
        g1-fitting-lib
        CONAN_PKG::eigen
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

    add_executable(test-clothoid-fit
        src/g1_fitting_test.cpp
        src/clothoid_test.cpp
    )
    set_target_properties(test-clothoid-fit PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-clothoid-fit
      PRIVATE
        g1-fitting-lib
        CONAN_PKG::gtest
        CONAN_PKG::eigen
        CONAN_PKG::cloe-runtime
        CONAN_PKG::cloe-models
    )
    gtest_add_tests(TARGET test-clothoid-fit)
endif()
