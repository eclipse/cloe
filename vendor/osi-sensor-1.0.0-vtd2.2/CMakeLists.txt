cmake_minimum_required(VERSION 3.7)

project(OSMPDummySensor
    LANGUAGES CXX
)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

# Compiler definitions
string(TIMESTAMP FMUTIMESTAMP UTC)
string(MD5 FMUGUID modelDescription.in.xml)
configure_file(modelDescription.in.xml modelDescription.xml @ONLY)
find_library(VTD_OSI_LIBRARY NAME libopen_simulation_interface.so
    PATHS "${CONAN_VTD_ROOT}/Data/Setups/Standard.OSI3/Bin/"
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
