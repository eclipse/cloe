#include <cloe/python/python_data_broker_adapter.hpp>

#include <optional>

namespace cloe::py {

void PythonDataBrokerAdapter::bind_signal(SignalPtr signal, std::string_view signal_name,
                                          std::string_view lua_name) {
  auto type = std::type_index(*signal->type());
  auto iter = bindings_->find(type);
  if (iter == bindings_->end()) {
    throw std::runtime_error(
        fmt::format("DataBroker: <internal logic error>: Lua type binding "
                    "for type \"{}\" not implemented", signal->type()->name()));
  }
  const auto& adapter = iter->second;
  adapter(signal, lua_name);
}
void PythonDataBrokerAdapter::bind(std::string_view signals_name) {
  // not needed!!
}
PythonDataBrokerAdapter::PythonDataBrokerAdapter()
    : bindings_(std::make_unique<std::map<std::type_index, SignalAdapter>>()), signals_(std::make_unique<Signals>()) {
  declare<bool>();
  declare<int8_t>();
  declare<uint8_t>();
  declare<int16_t>();
  declare<uint16_t>();
  declare<int32_t>();
  declare<uint32_t>();
  declare<int64_t>();
  declare<uint64_t>();
  declare<float>();
  declare<double>();
  declare<std::optional<bool>>();
  declare<std::optional<int8_t>>();
  declare<std::optional<uint8_t>>();
  declare<std::optional<int16_t>>();
  declare<std::optional<uint16_t>>();
  declare<std::optional<int32_t>>();
  declare<std::optional<uint32_t>>();
  declare<std::optional<int64_t>>();
  declare<std::optional<uint64_t>>();
  declare<std::optional<float>>();
  declare<std::optional<double>>();
}
const Signals& PythonDataBrokerAdapter::signals() const { return *signals_; }
}
