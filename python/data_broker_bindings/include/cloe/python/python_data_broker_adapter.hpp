#pragma once

#include "cloe/data_broker_binding.hpp"
#include "signals.hpp"

#include <typeindex>
#include <functional>

namespace cloe::py {

class PythonDataBrokerAdapter : public cloe::databroker::DataBrokerBinding {
 public:
  using SignalAdapter = std::function<void(const SignalPtr&, std::string_view)>;

  PythonDataBrokerAdapter();
  ~PythonDataBrokerAdapter() override = default;

  PythonDataBrokerAdapter(const PythonDataBrokerAdapter&) = delete;
  PythonDataBrokerAdapter& operator=(const PythonDataBrokerAdapter&) = delete;
  PythonDataBrokerAdapter(PythonDataBrokerAdapter&&) = default;
  PythonDataBrokerAdapter& operator=(PythonDataBrokerAdapter&&) = default;

  void bind_signal(SignalPtr signal, std::string_view signal_name,
                   std::string_view python_name) override;
  void bind(std::string_view signals_name) override;
  const Signals &signals() const;

  template<typename T>
  void declare() {
    assert_static_type<T>();
    using compatible_type = databroker::compatible_base_t<T>;

    // Check whether this type was already processed, if not declare it and store an adapter function in bindings_
    std::type_index type{typeid(compatible_type)};
    auto iter = bindings_->find(type);
    if (iter == bindings_->end()) {
      // Check whether this type was already declared to the Lua-VM, if not declare it
      auto declared_types_iter = declared_types().find(type);
      if (declared_types_iter == declared_types().end()) {
        declared_types()[type] = true;
      }

      // Store adapter function
      bindings_->emplace(type, [this](const SignalPtr& signal, std::string_view lua_name) {
        // Subscribe to the value-changed event to indicate the signal is used
        signal->subscribe<T>([](const T&) {});
        // Implement the signal as a property in Lua
        signals_->bind<T>(signal, lua_name);
      });
    }
  }

 private:
  std::unique_ptr<std::map<std::type_index, SignalAdapter>> bindings_;
  std::unique_ptr<Signals> signals_;
};
}
