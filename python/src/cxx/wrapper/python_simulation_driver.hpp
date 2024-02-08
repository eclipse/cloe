#pragma once

#include "python_data_broker_adapter.hpp"
#include "python_function.hpp"
#include "simulation_driver.hpp"

#include <tuple>
#include <vector>

namespace cloe::py {

namespace detail {
struct TriggerDescription {
  std::string label;
  nlohmann::json eventDescription;
  PythonFunction::CallbackFunction action;
  bool sticky;
};
}

class PythonSimulationDriver final : public engine::SimulationDriver {
 public:
  explicit PythonSimulationDriver(PythonDataBrokerAdapter *adapter);
  PythonSimulationDriver(PythonSimulationDriver&&) = default;
  PythonSimulationDriver& operator=(PythonSimulationDriver&&) = default;
  PythonSimulationDriver(const PythonSimulationDriver&) = delete;
  PythonSimulationDriver& operator=(const PythonSimulationDriver&) = delete;
  ~PythonSimulationDriver() final = default;

  void initialize(const engine::SimulationSync& sync, engine::Coordinator& scheduler) override;
  void register_action_factories(Registrar& registrar) override;
  void alias_signals(DataBroker& dataBroker) override;
  void bind_signals(DataBroker& dataBroker) override;
  std::vector<cloe::TriggerPtr> yield_pending_triggers() override;
  databroker::DataBrokerBinding* data_broker_binding() override;
  [[nodiscard]] nlohmann::json produce_report() const override;

  void add_require_signal(std::string_view signal_name);
  void add_signal_alias(std::string_view signal_name, std::string_view alias);
  void add_trigger(std::string_view label, const nlohmann::json &eventDescription,
                   const PythonFunction::CallbackFunction &action, bool sticky);

 private:
  PythonDataBrokerAdapter *adapter_;
  std::vector<detail::TriggerDescription> pending_triggers_{};
  std::vector<std::string> require_signals_ {};
  std::vector<std::tuple<std::string, std::string>> signal_aliases_ {};
};

}
