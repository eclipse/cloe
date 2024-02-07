#include "python_data_broker_adapter.hpp"

namespace cloe::py {

void PythonDataBrokerAdapter::bind_signal(SignalPtr signal, std::string_view signal_name,
                                          std::string_view lua_name) {}
void PythonDataBrokerAdapter::bind(std::string_view signals_name) {}
}
