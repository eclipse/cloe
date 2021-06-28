cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(cloe_engine LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Executable ---------------------------------------------------------
add_executable(fable_example
    src/main.cpp
)
set_target_properties(fable_example PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME fable_example
)

target_link_libraries(fable_example
  PRIVATE
    CONAN_PKG::fable
    CONAN_PKG::cli11
)

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS fable_example
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)
