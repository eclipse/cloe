#include "basic.hpp"
#include <cloe/python/python_data_broker_adapter.hpp>
#include <pybind11/pybind11.h>

PYBIND11_MODULE(_basic_bindings, m) {

  pybind11::class_<cloe::controller::basic::AccConfiguration>(m, "AccConfiguration")
      .def_readonly("ego_sensor", &cloe::controller::basic::AccConfiguration::ego_sensor)
      .def_readonly("world_sensor", &cloe::controller::basic::AccConfiguration::world_sensor)
      .def_readonly("latlong_actuator", &cloe::controller::basic::AccConfiguration::latlong_actuator)
      .def_readonly("limit_deceleration", &cloe::controller::basic::AccConfiguration::limit_deceleration)
      .def_readonly("derivative_factor_speed_control", &cloe::controller::basic::AccConfiguration::kd)
      .def_readonly("proportional_factor_speed_control", &cloe::controller::basic::AccConfiguration::kp)
      .def_readonly("integral_factor_speed_control", &cloe::controller::basic::AccConfiguration::ki)
      .def_readonly("derivative_factor_dist_control", &cloe::controller::basic::AccConfiguration::kd_m)
      .def_readonly("proportional_factor_dist_control", &cloe::controller::basic::AccConfiguration::kp_m)
      .def_readonly("integral_factor_dist_control", &cloe::controller::basic::AccConfiguration::ki_m);

  m.def("declare", [](cloe::py::PythonDataBrokerAdapter &adapter) {
    adapter.declare<cloe::controller::basic::AccConfiguration>();
  });

}
