cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(vtd_api LANGUAGES CXX VERSION 2.2.0)

add_library(vtd-api
    src/Develop/Framework/RDBHandler/RDBHandler.cc
)
add_library(vtd::api ALIAS vtd-api)
target_include_directories(vtd-api
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src/Develop/Framework/inc>"
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src/Develop/Framework/RDBHandler>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
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
        ${CMAKE_BINARY_DIR}/src/Develop/Framework/inc/
        ${CMAKE_BINARY_DIR}/src/Develop/Framework/RDBHandler/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PATTERN "*.cc" EXCLUDE
)
