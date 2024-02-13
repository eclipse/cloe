//
// Created by ohf4fe on 08.02.24.
//
#include "python_function.hpp"
#include <pybind11/pybind11.h>

#include <utility>
#include <memory>

namespace cloe::py {
PythonAction::PythonAction(CallbackFunction py_fun, std::string_view name)
    : cloe::Action(std::string(name)), py_fun_(std::move(py_fun)) {}
ActionPtr PythonAction::clone() const { return std::make_unique<PythonAction>(*this); }
CallbackResult PythonAction::operator()(const Sync& sync, TriggerRegistrar& /*registrar*/) {
  try {
    // no need to acquire gil, is automatically done by pybind
    return py_fun_(&sync);
  } catch (const std::exception &e) {
    throw cloe::Error("error executing Python function: {}", e.what());
  }
}
void PythonAction::to_json(Json& j) const { j = {}; }

}
