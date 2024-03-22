#pragma once

#include "cloe_fwd.hpp"

#include <unordered_map>
#include <typeindex>
#include <string_view>

namespace cloe::databroker {

class DataBrokerBinding {
 public:

  DataBrokerBinding() = default;
  DataBrokerBinding(const DataBrokerBinding&) = default;
  DataBrokerBinding(DataBrokerBinding&&) = default;
  DataBrokerBinding& operator=(const DataBrokerBinding&) = default;
  DataBrokerBinding& operator=(DataBrokerBinding&&) = default;
  virtual ~DataBrokerBinding() = default;

  virtual void bind_signal(SignalPtr signal, std::string_view signal_name, std::string_view lua_name) = 0;
  virtual void bind(std::string_view signals_name) = 0;

 protected:
  auto &declared_types() { return declared_types_; }
  const auto &declared_types() const { return declared_types_; }


 private:
  std::unordered_map<std::type_index, bool> declared_types_{};
};

}
