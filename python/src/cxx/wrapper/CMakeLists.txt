find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(_cloe_bindings
        bindings.cpp
        json.hpp
        json.cpp
        python_simulation_driver.hpp
        python_simulation_driver.cpp
        python_function.hpp
        python_function.cpp
)
target_link_libraries(_cloe_bindings PUBLIC cloe::stacklib cloe::enginelib data_broker_binding)
# todo this should be exposed by engine stacklib/enginelib via conan and/or root project
target_include_directories(_cloe_bindings PRIVATE ${PROJECT_SOURCE_DIR}/engine/src)
install(TARGETS _cloe_bindings
        LIBRARY
        DESTINATION ${CMAKE_INSTALL_PREFIX}/python/cloe
)
