cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(vtd_api LANGUAGES CXX VERSION 2022.3)

add_library(vtd-api
    src/Develop/Communication/VtdFramework/VtdToolkit/src/Common/RDBHandler.cc
)
add_library(vtd::api ALIAS vtd-api)
target_include_directories(vtd-api
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src/Develop/Communication/VtdFramework/VtdToolkit/include>"
    add_library(vtd::api ALIAS vtd-api)
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS vtd-api
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        add_library(vtd::api ALIAS vtd-api)
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY
        ${CMAKE_BINARY_DIR}/src/Develop/Communication/VtdFramework/VtdToolkit/include/VtdToolkit
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
