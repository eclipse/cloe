cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(OSMPDummySensor LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Input Variables
set(VTD_ROOT "" CACHE STRING "Path to VTD root directory")

# Dependencies
find_package(open_simulation_interface REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_library(VTD_OSI_LIBRARY NAME libopen_simulation_interface.so
    PATHS "${VTD_ROOT}/Data/Setups/Standard.OSI3/Bin/"
    NO_DEFAULT_PATH
)
if(NOT VTD_OSI_LIBRARY)
    message(WARNING "VTD OSI library not found. Using external package.")
    set(VTD_OSI_LIBRARY open_simulation_interface::libopen_simulation_interface)
endif()

# Configuration
string(TIMESTAMP FMUTIMESTAMP UTC)
string(MD5 FMUGUID modelDescription.in.xml)
configure_file(modelDescription.in.xml modelDescription.xml @ONLY)

# Library ------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED
    OSMPDummySensor.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
)
message(STATUS "protobuf_FOUND: ${protobuf_FOUND}")
message(STATUS "protobuf_INCLUDE_DIRS: ${protobuf_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME}
    PUBLIC
        ../includes
        ${open_simulation_interface_INCLUDE_DIRS}
        ${protobuf_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        protobuf::libprotobuf
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
