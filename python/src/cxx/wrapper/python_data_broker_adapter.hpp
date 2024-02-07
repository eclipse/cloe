#pragma once

#include "cloe/data_broker_binding.hpp"
namespace cloe::py {

class PythonDataBrokerAdapter : public cloe::databroker::DataBrokerBinding {
 public:
  PythonDataBrokerAdapter() = default;
  ~PythonDataBrokerAdapter() override = default;

  PythonDataBrokerAdapter(const PythonDataBrokerAdapter&) = delete;
  PythonDataBrokerAdapter& operator=(const PythonDataBrokerAdapter&) = delete;
  PythonDataBrokerAdapter(PythonDataBrokerAdapter&&) = default;
  PythonDataBrokerAdapter& operator=(PythonDataBrokerAdapter&&) = default;

  void bind_signal(SignalPtr signal, std::string_view signal_name,
                   std::string_view python_name) override;
  void bind(std::string_view signals_name) override;
};
}
