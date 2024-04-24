#pragma once

#include <cloe/simulation_driver.hpp>
#include <cloe/coordinator.hpp>
#include <cloe/databroker/data_broker_lua_binding.hpp>

#include <sol/state.hpp>   // for state

#include <unordered_map>
#include <typeindex>
#include <vector>

namespace cloe {

class LuaSimulationDriver final : public cloe::SimulationDriver {
 public:
  explicit LuaSimulationDriver(std::unique_ptr<sol::state> lua);
  LuaSimulationDriver(LuaSimulationDriver&&) = default;
  LuaSimulationDriver& operator=(LuaSimulationDriver&&) = default;
  LuaSimulationDriver(const LuaSimulationDriver&) = delete;
  LuaSimulationDriver& operator=(const LuaSimulationDriver&) = delete;
  ~LuaSimulationDriver() final;

  void initialize(const Sync &sync, coordinator::Coordinator& scheduler, DataBroker &db) override;
  void register_action_factories(Registrar& registrar) override;
  void alias_signals(DataBroker& dataBroker) override;
  void bind_signals(DataBroker& dataBroker) override;

  [[nodiscard]] nlohmann::json produce_report() const override;

  std::vector<TriggerPtr> yield_pending_triggers() override;

  static ActionPtr make_action(cloe::DriverTriggerFactory& factory, const sol::object& lua);
  static TriggerPtr make_trigger(cloe::DriverTriggerFactory& factory, const sol::table& tbl);

  databroker::LuaDataBrokerBinding* data_broker_binding() override;

  sol::table register_lua_table();

 private:
  std::unique_ptr<sol::state> lua_;
  std::unique_ptr<databroker::LuaDataBrokerBinding> data_broker_binding_;
};

}
