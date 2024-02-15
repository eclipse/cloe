cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
find_package(pybind11 CONFIG REQUIRED)

set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
if(CLOE_FIND_PACKAGES)
    find_package(cloe-runtime REQUIRED)
endif()

add_library(data_broker_binding SHARED src/python_data_broker_adapter.cpp src/signals.cpp)
add_library(cloe::databroker-bindings ALIAS data_broker_binding)
target_include_directories(data_broker_binding PUBLIC include)
target_link_libraries(data_broker_binding PUBLIC cloe::runtime pybind11::pybind11)
set_target_properties(data_broker_binding PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS data_broker_binding DESTINATION ${CMAKE_INSTALL_LIBDIR})
