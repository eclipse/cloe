cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(vtd_api LANGUAGES CXX VERSION 2.2.0)

add_library(vtd-api
    src/Develop/Framework/RDBHandler/RDBHandler.cc
)
add_library(vtd::api ALIAS vtd-api)
target_include_directories(vtd-api
  PUBLIC
    src/Develop/Framework/inc
    src/Develop/Framework/RDBHandler
)

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS vtd-api
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
