#pragma once

#include "data_broker_signal.hpp"

#include "cloe_fwd.hpp"

#include <unordered_map>
#include <typeindex>
#include <utility>

namespace cloe::databroker {

class DataBrokerBinding {
 public:

  virtual void bind_signal(SignalPtr signal, std::string_view signal_name, std::string_view lua_name) = 0;
  virtual void bind(std::string_view signals_name) = 0;

 protected:
  auto &declared_types() { return declared_types_; }
  const auto &declared_types() const { return declared_types_; }

 private:
  std::unordered_map<std::type_index, bool> declared_types_{};
};

}
