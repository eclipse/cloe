cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(vtd_api LANGUAGES CXX)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(target vtd_api)

add_library(${target}
    VTD.2.2/Develop/Framework/RDBHandler/RDBHandler.cc
)
target_include_directories(${target}
  PUBLIC
    VTD.2.2/Develop/Framework/inc
    VTD.2.2/Develop/Framework/RDBHandler
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
        VTD.2.2/Develop/Framework/inc/
        VTD.2.2/Develop/Framework/RDBHandler/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
