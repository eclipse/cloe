cmake_minimum_required(VERSION 3.7)

project(open_simulation_interface
    VERSION 3.2.0
    LANGUAGES CXX
)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

find_package(protoc CONFIG REQUIRED)

set(VERSION_MAJOR 3)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
configure_file(osi_version.proto.in ${CMAKE_CURRENT_SOURCE_DIR}/osi_version.proto)

add_library(${PROJECT_NAME}
    osi_version.proto
    osi_common.proto
    osi_datarecording.proto
    osi_detectedtrafficsign.proto
    osi_detectedtrafficlight.proto
    osi_detectedroadmarking.proto
    osi_detectedlane.proto
    osi_detectedobject.proto
    osi_detectedoccupant.proto
    osi_environment.proto
    osi_groundtruth.proto
    osi_hostvehicledata.proto
    osi_trafficsign.proto
    osi_trafficlight.proto
    osi_roadmarking.proto
    osi_lane.proto
    osi_featuredata.proto
    osi_object.proto
    osi_occupant.proto
    osi_sensordata.proto
    osi_sensorviewconfiguration.proto
    osi_sensorspecific.proto
    osi_sensorview.proto
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CONAN_PKG::protobuf
)
protobuf_generate(TARGET ${PROJECT_NAME}
    OUT_VAR protobuf_generated_files
    IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install generated headers
foreach(_file ${protobuf_generated_files})
    if(_file MATCHES "h$")
        list(APPEND protobuf_generated_headers ${_file})
    endif()
endforeach()
install(
    FILES ${protobuf_generated_headers}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
