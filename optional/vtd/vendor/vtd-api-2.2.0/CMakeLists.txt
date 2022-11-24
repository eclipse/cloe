cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(vtd_api LANGUAGES CXX VERSION 2.2.0)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(target vtd_api)

add_library(${target}
    src/Develop/Framework/RDBHandler/RDBHandler.cc
)
target_include_directories(${target}
  PUBLIC
    src/Develop/Framework/inc
    src/Develop/Framework/RDBHandler
)

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${target}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY
        src/Develop/Framework/inc/
        src/Develop/Framework/RDBHandler/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
