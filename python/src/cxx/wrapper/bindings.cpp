#include "json.hpp"
#include "python_simulation_driver.hpp"

#include "lua_setup.hpp"
#include "simulation.hpp"
#include "stack_factory.hpp"

#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>

PYBIND11_MODULE(_cloe_bindings, m) {
  namespace py = pybind11;
  m.doc() = "the cloe python binding";
  {
    py::class_<cloe::Stack> stack(m, "Stack");
    stack.def(py::init([]() {
      cloe::StackOptions stackOptions{};
      stackOptions.environment = std::make_unique<fable::Environment>();
      stackOptions.environment->set(CLOE_SIMULATION_UUID_VAR, "123"); // todo :( doesn't work without
      stackOptions.plugin_paths.emplace_back("/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib/cloe"); // todo remove
      // todo why can't i just create a new stack with its default c'tor?
      return cloe::new_stack(stackOptions);
    }));
    stack.def("merge", [](cloe::Stack &self, const py::dict &d, const std::string &file = "") {
      const auto json = cloe::py::dict2json(d);
      fable::Conf conf (json, file);
      self.from_conf(conf);
    }, py::arg("d"), py::arg("file") = "");
  }
  {
    py::class_<cloe::py::PythonDataBrokerAdapter> clazz (m, "DataBrokerAdapter");
    clazz.def(py::init<>());
  }
  {
    py::class_<cloe::py::PythonSimulationDriver> clazz (m, "SimulationDriver");
    clazz.def(py::init<cloe::py::PythonDataBrokerAdapter*>());
    clazz.def("add_signal_alias", &cloe::py::PythonSimulationDriver::add_signal_alias);
  }
  {
    py::class_<engine::Simulation> sim (m, "Simulation");
    sim.def(py::init([](cloe::Stack stack, cloe::py::PythonSimulationDriver &driver, const std::string &uuid) {
      cloe::LuaOptions opts {};
      opts.environment = std::make_unique<fable::Environment>();
      return engine::Simulation {std::move(stack), driver, uuid};
    }), py::arg("stack"), py::arg("driver"), py::arg("uuid"));
    // todo hooks!, store in ptr
    // todo is sim arg uuid == stack options uuid?
    sim.def("run", [](engine::Simulation &self) {
      return self.run();
    });
    sim.def("wait_until", [](engine::Simulation &self,
                             const std::function<bool(const cloe::Sync&)> &condition) {
      // todo impl, add timeout parameter
      try {
        py::cast(self); // todo check if (and what) this throws if type can not be cast
      } catch(...) {

      }
    });
  }
  {
    py::class_<engine::SimulationResult>(m, "SimulationResult")
        .def_readonly("uuid", &engine::SimulationResult::uuid)
        .def_readonly("sync", &engine::SimulationResult::sync)
        .def_readonly("elapsed", &engine::SimulationResult::elapsed);
    // todo expose remaining members
  }
  {
    py::class_<cloe::Sync> sync (m, "Sync");
    sync.def_property_readonly("step", &cloe::Sync::step);
    sync.def_property_readonly("step_width", &cloe::Sync::step_width);
    sync.def_property_readonly("time", &cloe::Sync::time);
    sync.def_property_readonly("eta", &cloe::Sync::eta);
    sync.def_property_readonly("realtime_factor", &cloe::Sync::realtime_factor);
    sync.def_property_readonly("is_realtime_factor_unlimited", &cloe::Sync::is_realtime_factor_unlimited);
    sync.def_property_readonly("achievable_realtime_factor", &cloe::Sync::achievable_realtime_factor);
  }
}
