#include "python_simulation_driver.hpp"
#include "cloe/model.hpp"

namespace cloe::py {

void PythonSimulationDriver::alias_signals(DataBroker& dataBroker) {
  for (const auto& [signal, alias] : signal_aliases_) {
    dataBroker.alias(signal, alias);
  }
}

void PythonSimulationDriver::initialize(const engine::SimulationSync& sync,
                                        engine::Coordinator& scheduler) {
  // todo not sure if needed in python
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
      throw cloe::ModelError("Binding signals to Lua failed with above error. Aborting.");
    }
  }
  //adapter_->bind("signals", (*lua_)); // todo??
}
std::vector<cloe::TriggerPtr> PythonSimulationDriver::yield_pending_triggers() {
  std::vector<cloe::TriggerPtr> result {};
  std::swap(pending_triggers_, result);
  return result;
}

databroker::DataBrokerBinding* PythonSimulationDriver::data_broker_binding() { return adapter_; }
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
void PythonSimulationDriver::add_trigger(
    std::string_view label, const nlohmann::json& eventDescription,
    const PythonFunction::CallbackFunction& action, bool sticky) {
  pending_triggers_.emplace_back(std::make_unique<cloe::Trigger>(
      std::string(label), cloe::Source::DRIVER,
      trigger_factory().make_event(fable::Conf{eventDescription}),
      std::make_unique<PythonFunction>(action, "python_function"))
  );
  pending_triggers_.back()->set_sticky(sticky);
}

}
