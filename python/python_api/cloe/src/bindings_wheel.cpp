#include <cloe/component/wheel.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_cloe_wheel(pybind11::module_ &m) {
  py::class_<cloe::Wheel>(m, "Wheel")
      .def_readonly("rotation", &cloe::Wheel::rotation)
      .def_readonly("velocity", &cloe::Wheel::velocity)
      .def_readonly("spring_compression", &cloe::Wheel::spring_compression);
}
