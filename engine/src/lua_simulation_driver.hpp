#pragma once

#include "simulation_driver.hpp"

#include <sol/state.hpp>   // for state

namespace engine {

class LuaSimulationDriver final : public SimulationDriver {
 public:
  explicit LuaSimulationDriver(std::unique_ptr<sol::state> lua);
  LuaSimulationDriver(LuaSimulationDriver&&) = default;
  LuaSimulationDriver& operator=(LuaSimulationDriver&&) = default;
  LuaSimulationDriver(const LuaSimulationDriver&) = delete;
  LuaSimulationDriver& operator=(const LuaSimulationDriver&) = delete;
  ~LuaSimulationDriver() final = default;

  void initialize(const SimulationSync &sync, Coordinator& scheduler) override;
  void register_action_factories(cloe::Registrar& registrar) override;
  void alias_signals(cloe::DataBroker& dataBroker) override;
  void bind_signals(cloe::DataBroker& dataBroker) override;

  nlohmann::json produce_report() const override;

 private:
  std::unique_ptr<sol::state> lua_;
};

}
