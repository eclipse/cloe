#pragma once

#include <cloe/python/python_data_broker_adapter.hpp>
#include <cloe/simulation_driver.hpp>

#include <tuple>
#include <vector>

namespace cloe::py {

using CallbackFunction = std::function<CallbackResult(const cloe::Sync*)>;

namespace detail {
struct TriggerDescription {
  std::string label;
  nlohmann::json eventDescription;
  CallbackFunction action;
  bool sticky;
};
}

class PythonSimulationDriver final : public cloe::SimulationDriver {
 public:
  explicit PythonSimulationDriver(PythonDataBrokerAdapter *adapter);
  PythonSimulationDriver(PythonSimulationDriver&&) = default;
  PythonSimulationDriver& operator=(PythonSimulationDriver&&) = default;
  PythonSimulationDriver(const PythonSimulationDriver&) = delete;
  PythonSimulationDriver& operator=(const PythonSimulationDriver&) = delete;
  ~PythonSimulationDriver() final = default;

  void initialize(const cloe::Sync& sync, cloe::coordinator::Coordinator& scheduler, cloe::DataBroker &dataBroker) override;
  void register_action_factories(Registrar& registrar) override;
  void alias_signals(DataBroker& dataBroker) override;
  void bind_signals(DataBroker& dataBroker) override;
  std::vector<cloe::TriggerPtr> yield_pending_triggers() override;
  PythonDataBrokerAdapter* data_broker_binding() override;
  [[nodiscard]] nlohmann::json produce_report() const override;

  void add_require_signal(std::string_view signal_name);
  void add_signal_alias(std::string_view signal_name, std::string_view alias);
  void register_trigger(std::string_view label, const nlohmann::json &eventDescription,
                        const CallbackFunction &action, bool sticky);
  void add_trigger(const cloe::Sync &sync, std::string_view label,
                   const nlohmann::json &eventDescription,
                   const CallbackFunction &action, bool sticky);

  [[nodiscard]] std::vector<std::string> available_signals() const;

  pybind11::module_ &extension_module();

 private:

  [[nodiscard]] std::unique_ptr<cloe::Trigger> trigger_description_to_trigger(const detail::TriggerDescription &) const;

  PythonDataBrokerAdapter *adapter_;
  std::vector<detail::TriggerDescription> pending_triggers_{};
  std::vector<std::string> require_signals_ {};
  std::vector<std::tuple<std::string, std::string>> signal_aliases_ {};
  cloe::coordinator::Coordinator* coordinator_ {};
  cloe::DataBroker* data_broker_ {};
};

}
