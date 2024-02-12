#pragma once

#include <cloe/simulation_driver.hpp>
#include <cloe/data_broker_lua_binding.hpp>
#include <cloe/coordinator.hpp>

#include <sol/state.hpp>   // for state

#include <unordered_map>
#include <typeindex>

namespace engine {

class LuaSimulationDriver final : public cloe::SimulationDriver {
 public:
  explicit LuaSimulationDriver(std::unique_ptr<sol::state> lua);
  LuaSimulationDriver(LuaSimulationDriver&&) = default;
  LuaSimulationDriver& operator=(LuaSimulationDriver&&) = default;
  LuaSimulationDriver(const LuaSimulationDriver&) = delete;
  LuaSimulationDriver& operator=(const LuaSimulationDriver&) = delete;
  ~LuaSimulationDriver() final = default;

  void initialize(const cloe::Sync &sync, cloe::coordinator::Coordinator& scheduler, cloe::DataBroker &db) override;
  void register_action_factories(cloe::Registrar& registrar) override;
  void alias_signals(cloe::DataBroker& dataBroker) override;
  void bind_signals(cloe::DataBroker& dataBroker) override;

  [[nodiscard]] nlohmann::json produce_report() const override;

  std::vector<cloe::TriggerPtr> yield_pending_triggers() override;

  static cloe::ActionPtr make_action(cloe::DriverTriggerFactory& factory, const sol::object& lua);
  static cloe::TriggerPtr make_trigger(cloe::DriverTriggerFactory& factory, const sol::table& tbl);

  cloe::databroker::DataBrokerBinding* data_broker_binding() override;

 private:
  std::unique_ptr<sol::state> lua_;
  std::unique_ptr<cloe::databroker::LuaDataBrokerBinding> data_broker_binding_;
};

}
