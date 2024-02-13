#include "basic.hpp"
#include <cloe/python/python_data_broker_adapter.hpp>
#include <pybind11/pybind11.h>

PYBIND11_MODULE(_basic_bindings, m) {

  pybind11::class_<cloe::controller::basic::AccConfiguration>(m, "AccConfiguration")
      .def_readonly("ego_sensor", &cloe::controller::basic::AccConfiguration::ego_sensor)
      .def_readonly("world_sensor", &cloe::controller::basic::AccConfiguration::world_sensor)
      .def_readonly("latlong_actuator", &cloe::controller::basic::AccConfiguration::latlong_actuator)
      .def_readonly("limit_deceleration", &cloe::controller::basic::AccConfiguration::limit_deceleration);

  m.def("declare", [](cloe::py::PythonDataBrokerAdapter &adapter) {
    adapter.declare<cloe::controller::basic::AccConfiguration>();
  });

}
