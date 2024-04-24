set(PYTHON_BINDINGS_LOCAL_DEV OFF CACHE BOOL "This sets the compiler output to the local source tree so
                                              the cloe python project can be simply imported.")
set(CLOE_FIND_PACKAGES ON CACHE BOOL "Call find_package() for cloe packages")
set(CLOE_ENGINE_VERSION ${CLOE_PROJECT_VERSION})
if(CLOE_FIND_PACKAGES)
    find_package(cloe-stacklib REQUIRED)
    find_package(cloe-simulation REQUIRED)
    find_package(cloe-databroker-bindings REQUIRED)
endif()

find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(_cloe_bindings
        bindings.cpp
        json.hpp
        json.cpp
        python_simulation_driver.hpp
        python_simulation_driver.cpp
        python_function.hpp
        python_function.cpp
        bindings_object.cpp
        bindings_wheel.cpp
)
set_target_properties(_cloe_bindings PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
if(PYTHON_BINDINGS_LOCAL_DEV)
    set_target_properties(_cloe_bindings
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.."
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.."
    )
endif()

target_link_libraries(_cloe_bindings PUBLIC cloe::stacklib cloe::databroker-bindings cloe::simulation)
target_include_directories(_cloe_bindings PRIVATE ${PROJECT_SOURCE_DIR}/engine/src)
install(TARGETS _cloe_bindings DESTINATION ${CMAKE_INSTALL_LIBDIR}/cloe/python/cloe)
