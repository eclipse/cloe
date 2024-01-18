#include "simulation.hpp"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(pycloe, m) {
  namespace py = pybind11;
  m.doc() = "the cloe python binding";
  auto engine = m.def_submodule("engine", "this module contains cloe-engine stuff");
  {
    py::class_<engine::Simulation> sim (m, "Simulation");
    // todo hooks!, store in ptr
    sim.def(py::init<const std::string&>());
    sim.def("run", [](engine::Simulation &self) {
      self.run();
    });
  }
}
