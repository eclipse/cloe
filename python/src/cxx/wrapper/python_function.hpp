#pragma once

#include "cloe/conf/action.hpp"
#include <functional>
#include <string_view>

namespace cloe::py {

class PythonFunction : public cloe::Action {
 public:
  using CallbackFunction = std::function<CallbackResult(const cloe::Sync*)>;
  PythonFunction(CallbackFunction py_fun, std::string_view name);
  PythonFunction(const PythonFunction &) = default;
  PythonFunction &operator=(const PythonFunction&) = default;
  PythonFunction(PythonFunction &&) = default;
  PythonFunction &operator=(PythonFunction&&) = default;
  ~PythonFunction() override = default;

  [[nodiscard]] ActionPtr clone() const override;

  CallbackResult operator()(const Sync &sync, TriggerRegistrar &registrar) override;
  void to_json(Json &j) const override;

 private:
  CallbackFunction py_fun_;
};

}
