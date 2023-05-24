cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(fable LANGUAGES CXX)

# NOTE: The variable FABLE_VERSION is used throughout this CMakeLists file
# and is supplied from Conan or by hand on the command line.
set(FABLE_VERSION "0.0.0-undefined" CACHE STRING "Fable version as MAJOR.MINOR.PATCH string")
set(FABLE_VERSION_U32 0 CACHE STRING "Fable version as (MAJOR<<16)|(MINOR<<8)|PATCH integer")

include(GNUInstallDirs)

# Library ------------------------------------------------------------
find_package(Boost COMPONENTS headers filesystem REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

file(GLOB fable_PUBLIC_HEADERS "include/**/*.hpp")
add_library(fable
    # find src -type f -name "*.cpp" \! -name "*_test.cpp"
    src/fable/conf.cpp
    src/fable/confable.cpp
    src/fable/json.cpp
    src/fable/schema.cpp
    src/fable/schema/boolean.cpp
    src/fable/schema/number.cpp
    src/fable/schema/struct.cpp
    src/fable/schema/string.cpp
    src/fable/schema/variant.cpp
    src/fable/schema/path.cpp
    src/fable/environment.cpp
    src/fable/utility.cpp

    # For IDE integration
    ${fable_PUBLIC_HEADERS}
)
add_library(fable::fable ALIAS fable)
set_target_properties(fable PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${FABLE_VERSION}
)
configure_file(src/fable/version.hpp.in include/fable/version.hpp @ONLY)
target_include_directories(fable
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(fable
  PUBLIC
    Boost::headers
    Boost::filesystem
    fmt::fmt
    nlohmann_json::nlohmann_json
)

option(FABLE_ALLOW_COMMENTS "Allow comments when parsing JSON?" ON)
if(FABLE_ALLOW_COMMENTS)
    target_compile_definitions(fable PRIVATE PARSE_JSON_WITH_COMMENTS=true)
else()
    target_compile_definitions(fable PRIVATE PARSE_JSON_WITH_COMMENTS=false)
endif()

# Testing ------------------------------------------------------------
include(CTest)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    add_executable(test-fable
        # find src -type f -name "*_test.cpp"
        src/fable/version_test.cpp
        src/fable/environment_test.cpp
        src/fable/schema/const_test.cpp
        src/fable/schema/custom_test.cpp
        src/fable/schema/enum_test.cpp
        src/fable/schema/factory_test.cpp
        src/fable/schema/factory_advanced_test.cpp
        src/fable/schema/ignore_test.cpp
        src/fable/schema/map_nested_test.cpp
        src/fable/schema/map_test.cpp
        src/fable/schema/number_test.cpp
        src/fable/schema/optional_test.cpp
        src/fable/schema/string_test.cpp
        src/fable/schema/struct_test.cpp
        src/fable/schema_test.cpp
    )
    set_target_properties(test-fable PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test-fable
        GTest::gtest
        GTest::gtest_main
        Boost::headers
        Boost::filesystem
        fable
    )
    gtest_add_tests(TARGET test-fable)
endif()

# Installation -------------------------------------------------------
install(TARGETS fable
    EXPORT fableTargets
    LIBRARY
    ARCHIVE
    RUNTIME
)
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT fableTargets
    NAMESPACE fable::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fable
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "fableConfigVersion.cmake"
    VERSION ${FABLE_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(FILES
    "cmake/fableConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/fableConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fable
)
