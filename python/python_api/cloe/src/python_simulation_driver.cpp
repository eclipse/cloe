#include "python_function.hpp"

#include <cloe/model.hpp>
#include "python_simulation_driver.hpp"
#include <cloe/coordinator.hpp>

namespace cloe::py {

void PythonSimulationDriver::alias_signals(DataBroker& dataBroker) {
  for (const auto& [signal, alias] : signal_aliases_) {
    dataBroker.alias(signal, alias);
  }
}

void PythonSimulationDriver::initialize(const cloe::Sync& /*sync*/,
                                        cloe::coordinator::Coordinator& scheduler,
                                        cloe::DataBroker &dataBroker) {
  coordinator_ = &scheduler;
  data_broker_ = &dataBroker;
}

void PythonSimulationDriver::register_action_factories(Registrar& registrar) {
  // todo: probably not needed, we don't want to run python scripts from within the engine
}
void PythonSimulationDriver::bind_signals(DataBroker& dataBroker) {
  const auto &registeredSignals = dataBroker.signals();
  for(const auto &signal : require_signals_) {
    if(registeredSignals.find(signal) != registeredSignals.end() ) {
      try {
        dataBroker.bind_signal(signal);
        logger()->info("Binding signal '{}' as '{}'.", signal, signal);
      } catch (const std::logic_error& ex) {
        logger()->error("Binding signal '{}' failed with error: {}", signal, ex.what());
      }
    } else {
      logger()->warn("Requested signal '{}' does not exist in DataBroker.", signal);
      throw cloe::ModelError("Binding signals to Python failed with above error. Aborting.");
    }
  }
}
std::vector<cloe::TriggerPtr> PythonSimulationDriver::yield_pending_triggers() {
  std::vector<cloe::TriggerPtr> result {};
  result.reserve(pending_triggers_.size());

  for(const auto &description : pending_triggers_) {
    result.emplace_back(trigger_description_to_trigger(description));
    result.back()->set_sticky(description.sticky);
  }
  pending_triggers_.clear();
  return result;
}

PythonDataBrokerAdapter* PythonSimulationDriver::data_broker_binding() { return adapter_; }
nlohmann::json PythonSimulationDriver::produce_report() const {
  // todo
  return {};
}
void PythonSimulationDriver::add_signal_alias(std::string_view signal_name,
                                              std::string_view alias) {
  signal_aliases_.emplace_back(signal_name, alias);
}
void PythonSimulationDriver::add_require_signal(std::string_view signal_name) {
  require_signals_.emplace_back(signal_name);
}

PythonSimulationDriver::PythonSimulationDriver(PythonDataBrokerAdapter* adapter)
    : adapter_(adapter) {}

void PythonSimulationDriver::register_trigger(
    std::string_view label, const nlohmann::json& eventDescription,
    const CallbackFunction& action, bool sticky) {
  if (coordinator_ != nullptr) {
    throw std::runtime_error("simulation is already running, use add_trigger.");
  }
  detail::TriggerDescription description {
      .label = std::string(label),
      .eventDescription = eventDescription,
      .action = action,
      .sticky = sticky
  };
  pending_triggers_.push_back(description);  // deferred trigger initialization
}
std::unique_ptr<cloe::Trigger> PythonSimulationDriver::trigger_description_to_trigger(
    const detail::TriggerDescription& description) const {
  auto result = std::make_unique<cloe::Trigger>(
      description.label, cloe::Source::DRIVER,
      trigger_factory().make_event(fable::Conf{description.eventDescription}),
      std::make_unique<PythonAction>(description.action, "python_function"));
  result->set_sticky(description.sticky);
  return result;
}
void PythonSimulationDriver::add_trigger(const Sync& sync, std::string_view label,
                                         const nlohmann::json& eventDescription,
                                         const PythonAction::CallbackFunction& action,
                                         bool sticky) {
  detail::TriggerDescription description {
      .label = std::string(label),
      .eventDescription = eventDescription,
      .action = action,
      .sticky = sticky
  };
  coordinator_->insert_trigger(sync, trigger_description_to_trigger(description));
}
std::vector<std::string> PythonSimulationDriver::available_signals() const {
  if(data_broker_ == nullptr){
    throw std::runtime_error("databroker not yet initialized");
  }
  std::vector<std::string> result {};
  result.reserve(data_broker_->signals().size());
  for(const auto &[key, _] : data_broker_->signals()) {
    result.emplace_back(key);
  }
  return result;
}

}
