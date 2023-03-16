cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(OSMPDummySensor
    LANGUAGES CXX
)

set(VTD_ROOT ${CONAN_VTD_ROOT} CACHE FILEPATH "Path to VTD root installation directory")

# Compiler definitions
string(TIMESTAMP FMUTIMESTAMP UTC)
string(MD5 FMUGUID modelDescription.in.xml)
configure_file(modelDescription.in.xml modelDescription.xml @ONLY)
find_library(VTD_OSI_LIBRARY NAME libopen_simulation_interface.so
    PATHS "${VTD_ROOT}/Data/Setups/Standard.OSI3/Bin/"
    NO_DEFAULT_PATH
)
if(NOT VTD_OSI_LIBRARY)
    message(WARNING "VTD OSI library not found. Using Conan package..")
    set(VTD_OSI_LIBRARY CONAN_PKG::open-simulation-interface)
endif()

# Library ------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED
    OSMPDummySensor.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ../includes
        $<BUILD_INTERFACE:${CONAN_INCLUDE_DIRS_PROTOBUF}>
        $<BUILD_INTERFACE:${CONAN_INCLUDE_DIRS_OPEN-SIMULATION-INTERFACE}>
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${VTD_OSI_LIBRARY}
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    FMU_SHARED_OBJECT
    PRIVATE_LOG_PATH="/tmp/OSMPDummySensor.log"
)

# Installation -------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
