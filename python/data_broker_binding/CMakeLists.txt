find_package(pybind11 CONFIG REQUIRED)

add_library(data_broker_binding SHARED src/python_data_broker_adapter.cpp src/signals.cpp)
target_include_directories(data_broker_binding PUBLIC include)
target_link_libraries(data_broker_binding PUBLIC cloe::runtime pybind11::pybind11)
