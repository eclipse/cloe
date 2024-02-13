#pragma once

#include <cloe/trigger.hpp>
#include <cloe/conf/action.hpp>

#include <functional>
#include <string_view>

namespace cloe::py {

class PythonAction : public cloe::Action {
 public:
  using CallbackFunction = std::function<CallbackResult(const cloe::Sync*)>;
  PythonAction(CallbackFunction py_fun, std::string_view name);
  PythonAction(const PythonAction &) = default;
  PythonAction &operator=(const PythonAction &) = default;
  PythonAction(PythonAction &&) = default;
  PythonAction &operator=(PythonAction &&) = default;
  ~PythonAction() override = default;

  [[nodiscard]] ActionPtr clone() const override;

  CallbackResult operator()(const Sync &sync, TriggerRegistrar &registrar) override;
  void to_json(Json &j) const override;

 private:
  CallbackFunction py_fun_;
};

}
