#include "json.hpp"
#include "lua_setup.hpp"
#include "simulation.hpp"
#include "stack_factory.hpp"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(_cloe_bindings, m) {
  namespace py = pybind11;
  m.doc() = "the cloe python binding";
  auto engine = m.def_submodule("engine", "this module contains cloe-engine stuff");
  {
    py::class_<cloe::Stack> stack(m, "Stack");
    stack.def(py::init([]() {
      // blergh!
      cloe::StackOptions stackOptions{};
      stackOptions.environment = std::make_unique<fable::Environment>();
      stackOptions.environment->set(CLOE_SIMULATION_UUID_VAR, "123"); // todo :( doesn't work without
      stackOptions.plugin_paths.emplace_back("/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib/cloe"); // todo remove
      // stackOptions.environment->set("CLOE_LOG_LEVEL", "trace"); // todo :( doesn't work without
      // todo why can't i just create a new stack with its default c'tor?
      cloe::Stack s = cloe::new_stack(stackOptions);
      // todo why do i have to call reset schema here, i get segfault otherwise :(
      s.reset_schema();
      return s;
    }));
    stack.def("merge", [](cloe::Stack &self, const py::dict &d, const std::string &file = "") {
      const auto json = cloe::py::dict2json(d);
      fable::Conf conf (json, file);
      self.from_conf(conf);
    }, py::arg("d"), py::arg("file") = "");
  }
  {
    m.def("create_sim", [](cloe::Stack stack, const std::string &uuid){
      // todo argh, engine isn't movable so i have to do it like this instead of defining python ctor
      //    (fix: lua state is unique ptr)
      cloe::LuaOptions opts {};
      opts.environment = std::make_unique<fable::Environment>();
      opts.environment->set("CLOE_LUA_PATH", ""); // todo: mandatory, otherwise segfault
      sol::state lua = cloe::new_lua(opts, stack);
      return std::make_unique<engine::Simulation>(std::move(stack), std::move(lua), uuid);
    }, py::arg("stack"), py::arg("uuid"));
    py::class_<engine::Simulation> sim (m, "Simulation");
    // todo hooks!, store in ptr
    // todo is sim arg uuid == stack options uuid?
    sim.def("run", [](engine::Simulation &self) {
      self.run();
    });
  }
}
