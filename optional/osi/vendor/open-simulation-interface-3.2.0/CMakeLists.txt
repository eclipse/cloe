cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(open_simulation_interface
    LANGUAGES CXX
)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

find_package(Protobuf REQUIRED)
# make sure that protobuf_generate_cpp finds the protobuf headers
set(Protobuf_IMPORT_DIRS ${Protobuf_INCLUDE_DIRS})

set(VERSION_MAJOR 3)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)
configure_file(osi_version.proto.in ${CMAKE_CURRENT_SOURCE_DIR}/osi_version.proto)

set(OSI_SOURCE_FILES
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
protobuf_generate_cpp(OSI_PROTO_SRCS OSI_PROTO_HDRS ${OSI_SOURCE_FILES})

add_library(${PROJECT_NAME}
    ${OSI_PROTO_SRCS}
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        # todo:  migrate to conan 2.0
        CONAN_PKG::protobuf
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install generated headers
install(
    FILES ${OSI_PROTO_HDRS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
