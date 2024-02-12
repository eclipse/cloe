#include "json.hpp"
#include "python_simulation_driver.hpp"

#include "lua_setup.hpp"
#include "simulation.hpp"
#include "stack_factory.hpp"

#include <pybind11/chrono.h>
#include <pybind11/eigen.h>
#include <pybind11/functional.h>
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
    clazz.def_property_readonly("signals", &cloe::py::PythonDataBrokerAdapter::signals);
  }
  {
    using Driver = cloe::py::PythonSimulationDriver;
    py::class_<Driver> clazz (m, "SimulationDriver");
    clazz.def(py::init<cloe::py::PythonDataBrokerAdapter*>());
    clazz.def("add_signal_alias", &Driver::add_signal_alias);
    clazz.def("register_trigger", [](Driver &self, std::string_view label, const py::dict& eventDescription,
                                const cloe::py::PythonFunction::CallbackFunction& action, bool sticky) {
                self.register_trigger(label, cloe::py::dict2json(eventDescription), action, sticky);
    });
    clazz.def("add_trigger", [](Driver &self, const cloe::Sync &sync, std::string_view label, const py::dict& eventDescription,
                                const cloe::py::PythonFunction::CallbackFunction& action, bool sticky) {
                self.add_trigger(sync, label, cloe::py::dict2json(eventDescription), action, sticky);
    });
  }
  {
    py::class_<engine::Simulation> sim (m, "Simulation");
    sim.def(py::init([](cloe::Stack stack, cloe::py::PythonSimulationDriver &driver, const std::string &uuid) {
      cloe::LuaOptions opts {};
      opts.environment = std::make_unique<fable::Environment>();
      return engine::Simulation {std::move(stack), driver, uuid};
    }), py::arg("stack"), py::arg("driver"), py::arg("uuid"));
    sim.def_property(
        "log_level",
        [](const engine::Simulation &self) {
          return cloe::logger::to_string(self.logger()->level());
        },
        [](engine::Simulation &self, std::string_view level) {
          self.logger()->set_level(cloe::logger::into_level(std::string(level)));
        });
    // todo hooks!, store in ptr
    // todo is sim arg uuid == stack options uuid?
    sim.def("run", [](engine::Simulation &self) {
      py::gil_scoped_release release_gil;
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
  {
    py::enum_<cloe::CallbackResult> clazz (m, "CallbackResult");
    clazz.value("Ok", cloe::CallbackResult::Ok);
    clazz.value("Unpin", cloe::CallbackResult::Unpin);
  }
}
