/*
 * Copyright 2024 Robert Bosch GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * \file simulation_state_probe.cpp
 */

#include <cloe/data_broker.hpp>   // for DataBroker
#include <cloe/vehicle.hpp>       // for Vehicle::component_names
#include <fable/utility/sol.hpp>  // for to_json for sol::object

#include "coordinator.hpp"         // for Coordinator::trigger_events, ...
#include "lua_api.hpp"             // for luat_cloe_engine_state
#include "server.hpp"              // for Server::endpoints
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine
#include "simulation_probe.hpp"    // for SimulationProbe

namespace engine {

std::map<std::string, std::string> dump_signals(const cloe::DataBroker& db) {
  std::map<std::string, std::string> report;

  for (const auto& [key, signal] : db.signals()) {
    // Find out if we are dealing with an alias or the actual signal.
    assert(!signal->names().empty());
    if (key != signal->names()[0]) {
      // We have an alias, because the name is at the first place.
      // FIXME: Direct coupling to implementation detail of Signal.
      report[key] = fmt::format("@alias {}", signal->names()[0]);
      continue;
    }

    // Get lua tag if possible
    const auto* tag = signal->metadata<cloe::LuaAutocompletionTag>();
    if (tag == nullptr) {
      report[key] = fmt::format("@field {}", key);
      continue;
    }
    report[key] = fmt::format("@field {} {} {}", key, to_string(tag->datatype), tag->text);
  }

  return report;
}

StateId SimulationMachine::Probe::impl(SimulationContext& ctx) {
  logger()->info("Probing simulation parameters.");

  ctx.outcome = SimulationOutcome::Probing;
  auto data = SimulationProbe();
  data.uuid = ctx.uuid;
  for (const auto& [name, plugin] : ctx.config.get_all_plugins()) {
    data.plugins[plugin->name()] = plugin->path();
  }
  for (const auto& [name, veh] : ctx.vehicles) {
    data.vehicles[name] = veh->component_names();
  }
  data.trigger_actions = ctx.coordinator->trigger_action_names();
  data.trigger_events = ctx.coordinator->trigger_event_names();
  data.http_endpoints = ctx.server->endpoints();
  data.signal_metadata = dump_signals(*ctx.db);
  data.test_metadata = sol::object(cloe::luat_cloe_engine_state(ctx.lua)["report"]["tests"]);

  ctx.probe = data;
  return DISCONNECT;
}

}  // namespace engine
