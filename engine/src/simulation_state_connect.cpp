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
 * \file simulation_state_connect.cpp
 */

#include <cloe/controller.hpp>                // for Controller
#include <cloe/data_broker.hpp>               // for DataBroker
#include <cloe/registrar.hpp>                 // for DirectCallback
#include <cloe/simulator.hpp>                 // for Simulator
#include <cloe/trigger/example_actions.hpp>   // for CommandFactory, BundleFactory, ...
#include <cloe/trigger/set_action.hpp>        // for DEFINE_SET_STATE_ACTION, SetDataActionFactory
#include <cloe/utility/resource_handler.hpp>  // for INCLUDE_RESOURCE, RESOURCE_HANDLER
#include <cloe/vehicle.hpp>                   // for Vehicle
#include <fable/utility.hpp>                  // for indent_string
#include <fable/utility/sol.hpp>              // for sol::object to_json

#include "coordinator.hpp"            // for register_usertype_coordinator
#include "lua_action.hpp"             // for LuaAction, ...
#include "lua_api.hpp"                // for to_json(json, sol::object)
#include "registrar.hpp"              // for Registrar::...
#include "server.hpp"                 // for Server::...
#include "simulation_actions.hpp"     // for StopFactory, ...
#include "simulation_context.hpp"     // for SimulationContext
#include "simulation_machine.hpp"     // for Connect, ...
#include "simulation_outcome.hpp"     // for SimulationOutcome
#include "utility/command.hpp"        // for CommandFactory
#include "utility/state_machine.hpp"  // for State, StateMachine

// PROJECT_SOURCE_DIR is normally exported by CMake during build, but it's not
// available for the linters, so we define a dummy value here for that case.
#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR ""
#endif

INCLUDE_RESOURCE(index_html, PROJECT_SOURCE_DIR "/webui/index.html");
INCLUDE_RESOURCE(favicon, PROJECT_SOURCE_DIR "/webui/cloe_16x16.png");
INCLUDE_RESOURCE(cloe_logo, PROJECT_SOURCE_DIR "/webui/cloe.svg");
INCLUDE_RESOURCE(bootstrap_css, PROJECT_SOURCE_DIR "/webui/bootstrap.min.css");

namespace engine {

std::string enumerate_simulator_vehicles(const cloe::Simulator& s) {
  std::stringstream buffer;
  auto n = s.num_vehicles();
  for (size_t i = 0; i < n; i++) {
    auto v = s.get_vehicle(i);
    buffer << fmt::format("{}: {}\n", i, v->name());
  }
  return buffer.str();
}

void handle_cloe_error(cloe::Logger logger, const cloe::Error& e) {
  if (e.has_explanation()) {
    logger->error("Note:\n{}", fable::indent_string(e.explanation(), "    "));
  }
}

StateId SimulationMachine::Connect::impl(SimulationContext& ctx) {
  logger()->info("Initializing simulation...");
  assert(ctx.config.is_valid());

  ctx.outcome = SimulationOutcome::NoStart;

  // 1. Initialize progress tracking
  ctx.progress.init_begin(6);
  auto update_progress = [&ctx](const char* str) {
    ctx.progress.init(str);
    ctx.server->refresh_buffer();
  };

  {  // 2. Initialize loggers
    update_progress("logging");

    for (const auto& c : ctx.config.logging) {
      c.apply();
    }
  }

  {  // 3. Initialize Lua
    auto types_tbl = sol::object(cloe::luat_cloe_engine_types(ctx.lua)).as<sol::table>();
    register_usertype_coordinator(types_tbl, ctx.sync);

    cloe::luat_cloe_engine_state(ctx.lua)["scheduler"] = std::ref(*ctx.coordinator);
  }

  {  // 4. Enroll endpoints and triggers for the server
    update_progress("server");

    auto rp = ctx.simulation_registrar();
    cloe::Registrar& r = *rp;

    // HTML endpoints:
    r.register_static_handler("/", RESOURCE_HANDLER(index_html, cloe::ContentType::HTML));
    r.register_static_handler("/index.html", cloe::handler::Redirect("/"));
    r.register_static_handler("/cloe_16x16.png", RESOURCE_HANDLER(favicon, cloe::ContentType::PNG));
    r.register_static_handler("/cloe.svg", RESOURCE_HANDLER(cloe_logo, cloe::ContentType::SVG));
    r.register_static_handler("/bootstrap.css",
                              RESOURCE_HANDLER(bootstrap_css, cloe::ContentType::CSS));

    // API endpoints:
    r.register_api_handler("/uuid", cloe::HandlerType::STATIC, cloe::handler::StaticJson(ctx.uuid));
    r.register_api_handler("/version", cloe::HandlerType::STATIC,
                           cloe::handler::StaticJson(ctx.version()));
    r.register_api_handler("/progress", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<SimulationProgress>(&ctx.progress));
    r.register_api_handler(
        "/configuration", cloe::HandlerType::DYNAMIC,
        [&ctx](const cloe::Request& q, cloe::Response& r) {
          std::string type = "active";
          auto m = q.query_map();
          if (m.count("type")) {
            type = m.at("type");
          }

          if (type == "active") {
            r.write(ctx.config.active_config());
          } else if (type == "input") {
            r.write(ctx.config.input_config());
          } else {
            r.bad_request(cloe::Json{
                {"error", "invalid type value"},
                {"fields", {{"type", "configuration output type, one of: active, input"}}},
            });
          }
        });
    r.register_api_handler("/simulation", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<SimulationSync>(&ctx.sync));
    r.register_api_handler("/statistics", cloe::HandlerType::BUFFERED,
                           cloe::handler::ToJson<SimulationStatistics>(&ctx.statistics));
    r.register_api_handler("/plugins", cloe::HandlerType::STATIC,
                           cloe::handler::StaticJson(ctx.plugin_ids()));

    // Coordinator & Server
    ctx.server->enroll(r);
    ctx.coordinator->enroll(r);

    // Events:
    ctx.callback_loop = r.register_event<events::LoopFactory>();
    ctx.callback_start = r.register_event<events::StartFactory>();
    ctx.callback_stop = r.register_event<events::StopFactory>();
    ctx.callback_success = r.register_event<events::SuccessFactory>();
    ctx.callback_failure = r.register_event<events::FailureFactory>();
    ctx.callback_reset = r.register_event<events::ResetFactory>();
    ctx.callback_pause = r.register_event<events::PauseFactory>();
    ctx.callback_resume = r.register_event<events::ResumeFactory>();
    ctx.callback_time = std::make_shared<events::TimeCallback>(
        logger(), [this, &ctx](const cloe::Trigger& t, cloe::Duration when) {
          static const std::vector<std::string> eta_names{"stop", "succeed", "fail", "reset"};
          auto name = t.action().name();
          for (std::string x : eta_names) {
            // Take possible namespacing of simulation actions into account.
            if (ctx.config.simulation.name) {
              x = *ctx.config.simulation.name;
              x += "/";
              x += x;
            }
            if (name == x) {
              // We are only interested in the earliest stop action.
              if (ctx.sync.eta() == cloe::Duration(0) || when < ctx.sync.eta()) {
                logger()->info("Set simulation ETA to {}s", cloe::Seconds{when}.count());
                ctx.sync.set_eta(when);
                ctx.progress.execution_eta = when;
              }
            }
          }
        });
    r.register_event(std::make_unique<events::TimeFactory>(), ctx.callback_time);
    r.register_event(std::make_unique<events::NextFactory>(),
                     std::make_shared<events::NextCallback>(ctx.callback_time));

    // Actions:
    r.register_action<actions::PauseFactory>(this->state_machine());
    r.register_action<actions::ResumeFactory>(this->state_machine());
    r.register_action<actions::StopFactory>(this->state_machine());
    r.register_action<actions::ResetFactory>(this->state_machine());
    r.register_action<actions::FailFactory>(this->state_machine());
    r.register_action<actions::SucceedFactory>(this->state_machine());
    r.register_action<actions::KeepAliveFactory>(&ctx);
    r.register_action<actions::RealtimeFactorFactory>(&ctx.sync);
    r.register_action<actions::ResetStatisticsFactory>(&ctx.statistics);
    r.register_action<actions::CommandFactory>(ctx.commander.get());
    r.register_action<actions::LuaFactory>(ctx.lua);

    // From: cloe/trigger/example_actions.hpp
    auto tr = ctx.coordinator->trigger_registrar(cloe::Source::TRIGGER);
    r.register_action<cloe::actions::BundleFactory>(tr);
    r.register_action<cloe::actions::InsertFactory>(tr);
    r.register_action<cloe::actions::LogFactory>();
    r.register_action<cloe::actions::PushReleaseFactory>(tr);
  }

  {  // 5. Initialize simulators
    update_progress("simulators");

    /**
     * Return a new Simulator given configuration c.
     */
    auto new_simulator = [&ctx](const cloe::SimulatorConf& c) -> std::unique_ptr<cloe::Simulator> {
      auto f = c.factory->clone();
      auto name = c.name.value_or(c.binding);
      for (auto d : ctx.config.get_simulator_defaults(name, f->name())) {
        f->from_conf(d.args);
      }
      auto x = f->make(c.args);
      ctx.now_initializing = x.get();

      // Configure simulator:
      auto r = ctx.registrar->with_trigger_prefix(name)->with_api_prefix(
          std::string("/simulators/") + name);
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    // Create and configure all simulators:
    for (const auto& c : ctx.config.simulators) {
      auto name = c.name.value_or(c.binding);
      assert(ctx.simulators.count(name) == 0);
      logger()->info("Configure simulator {}", name);

      try {
        ctx.simulators[name] = new_simulator(c);
      } catch (cloe::ModelError& e) {
        logger()->critical("Error configuring simulator {}: {}", name, e.what());
        return ABORT;
      }
    }

    auto r = ctx.simulation_registrar();
    r->register_api_handler("/simulators", cloe::HandlerType::STATIC,
                            cloe::handler::StaticJson(ctx.simulator_ids()));
  }

  {  // 6. Initialize vehicles
    update_progress("vehicles");

    /**
     * Return a new Component given vehicle v and configuration c.
     */
    auto new_component = [&ctx](cloe::Vehicle& v,
                                const cloe::ComponentConf& c) -> std::shared_ptr<cloe::Component> {
      // Create a copy of the component factory prototype and initialize it with the default stack arguments.
      auto f = c.factory->clone();
      auto name = c.name.value_or(c.binding);
      for (auto d : ctx.config.get_component_defaults(name, f->name())) {
        f->from_conf(d.args);
      }
      // Get input components, if applicable.
      std::vector<std::shared_ptr<cloe::Component>> from;
      for (const auto& from_comp_name : c.from) {
        if (!v.has(from_comp_name)) {
          return nullptr;
        }
        from.push_back(v.get<cloe::Component>(from_comp_name));
      }
      // Create the new component.
      auto x = f->make(c.args, std::move(from));
      ctx.now_initializing = x.get();

      // Configure component:
      auto r = ctx.registrar->with_trigger_prefix(name)->with_api_prefix(
          std::string("/components/") + name);
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    /**
     * Return a new Vehicle given configuration c.
     */
    auto new_vehicle = [&](const cloe::VehicleConf& c) -> std::shared_ptr<cloe::Vehicle> {
      static uint64_t gid = 1024;

      // Fetch vehicle prototype.
      std::shared_ptr<cloe::Vehicle> v;
      if (c.is_from_simulator()) {
        auto& s = ctx.simulators.at(c.from_sim.simulator);
        if (c.from_sim.is_by_name()) {
          v = s->get_vehicle(c.from_sim.index_str);
          if (!v) {
            throw cloe::ModelError("simulator {} has no vehicle by name {}", c.from_sim.simulator,
                                   c.from_sim.index_str)
                .explanation("Simulator {} has following vehicles:\n{}", c.from_sim.simulator,
                             enumerate_simulator_vehicles(*s));
          }
        } else {
          v = s->get_vehicle(c.from_sim.index_num);
          if (!v) {
            throw cloe::ModelError("simulator {} has no vehicle at index {}", c.from_sim.simulator,
                                   c.from_sim.index_num)
                .explanation("Simulator {} has following vehicles:\n{}", c.from_sim.simulator,
                             enumerate_simulator_vehicles(*s));
          }
        }
      } else {
        if (ctx.vehicles.count(c.from_veh)) {
          v = ctx.vehicles.at(c.from_veh);
        } else {
          // This vehicle depends on another that hasn't been create yet.
          return nullptr;
        }
      }

      // Create vehicle from prototype and configure the components.
      logger()->info("Configure vehicle {}", c.name);
      auto x = v->clone(++gid, c.name);
      ctx.now_initializing = x.get();

      std::set<std::string> configured;
      size_t n = c.components.size();
      while (configured.size() != n) {
        // Keep trying to create components until all have been created.
        // This is a poor-man's version of dependency resolution and has O(n^2)
        // complexity, which is acceptable given that the expected number of
        // components is usually less than 100.
        size_t m = configured.size();
        for (const auto& kv : c.components) {
          if (configured.count(kv.first)) {
            // This component has already been configured.
            continue;
          }

          auto k = new_component(*x, kv.second);
          if (k) {
            x->set_component(kv.first, std::move(k));
            configured.insert(kv.first);
          }
        }

        // Check that we are making progress.
        if (configured.size() == m) {
          // We have configured.size() != n and has not grown since going
          // through all Component configs. This means that we have some unresolved
          // dependencies. Find out which and abort.
          for (const auto& kv : c.components) {
            if (configured.count(kv.first)) {
              continue;
            }

            // We now have a component that has not been configured, and this
            // can only be the case if the dependency is not found.
            assert(kv.second.from.size() > 0);
            for (const auto& from_comp_name : kv.second.from) {
              if (x->has(from_comp_name)) {
                continue;
              }
              throw cloe::ModelError{
                  "cannot configure component '{}': cannot resolve dependency '{}'",
                  kv.first,
                  from_comp_name,
              };
            }
          }
        }
      }

      // Configure vehicle:
      auto r = ctx.registrar->with_trigger_prefix(c.name)->with_api_prefix(
          std::string("/vehicles/") + c.name);
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    // Create and configure all vehicles:
    size_t n = ctx.config.vehicles.size();
    while (ctx.vehicles.size() != n) {
      // Keep trying to create vehicles until all have been created.
      // This is a poor-man's version of dependency resolution and has O(n^2)
      // complexity, which is acceptable given that the expected number of
      // vehicles is almost always less than 10.
      size_t m = ctx.vehicles.size();
      for (const auto& c : ctx.config.vehicles) {
        if (ctx.vehicles.count(c.name)) {
          // This vehicle has already been configured.
          continue;
        }

        std::shared_ptr<cloe::Vehicle> v;
        try {
          v = new_vehicle(c);
        } catch (cloe::ModelError& e) {
          logger()->critical("Error configuring vehicle {}: {}", c.name, e.what());
          handle_cloe_error(logger(), e);
          return ABORT;
        }

        if (v) {
          ctx.vehicles[c.name] = std::move(v);
        }
      }

      // Check that we are making progress.
      if (ctx.vehicles.size() == m) {
        // We have ctx.vehicles.size() != n and has not grown since going
        // through all Vehicle configs. This means that we have some unresolved
        // dependencies. Find out which and abort.
        for (const auto& c : ctx.config.vehicles) {
          if (ctx.vehicles.count(c.name)) {
            continue;
          }

          // We now have a vehicle that has not been configured, and this can
          // only be the case if a vehicle dependency is not found.
          assert(c.is_from_vehicle());
          throw cloe::ModelError{
              "cannot configure vehicle '{}': cannot resolve dependency '{}'",
              c.name,
              c.from_veh,
          };
        }
      }
    }

    auto r = ctx.simulation_registrar();
    r->register_api_handler("/vehicles", cloe::HandlerType::STATIC,
                            cloe::handler::StaticJson(ctx.vehicle_ids()));
  }

  {  // 7. Initialize controllers
    update_progress("controllers");

    /**
     * Return a new Controller given configuration c.
     */
    auto new_controller =
        [&ctx](const cloe::ControllerConf& c) -> std::unique_ptr<cloe::Controller> {
      auto f = c.factory->clone();
      auto name = c.name.value_or(c.binding);
      for (auto d : ctx.config.get_controller_defaults(name, f->name())) {
        f->from_conf(d.args);
      }
      auto x = f->make(c.args);
      ctx.now_initializing = x.get();

      // Configure
      auto r = ctx.registrar->with_trigger_prefix(name)->with_api_prefix(
          std::string("/controllers/") + name);
      x->set_vehicle(ctx.vehicles.at(c.vehicle));
      x->connect();
      x->enroll(*r);

      ctx.now_initializing = nullptr;
      return x;
    };

    // Create and configure all controllers:
    for (const auto& c : ctx.config.controllers) {
      auto name = c.name.value_or(c.binding);
      assert(ctx.controllers.count(name) == 0);
      logger()->info("Configure controller {}", name);
      try {
        ctx.controllers[name] = new_controller(c);
      } catch (cloe::ModelError& e) {
        logger()->critical("Error configuring controller {}: {}", name, e.what());
        return ABORT;
      }
    }

    auto r = ctx.simulation_registrar();
    r->register_api_handler("/controllers", cloe::HandlerType::STATIC,
                            cloe::handler::StaticJson(ctx.controller_ids()));
  }

  {  // 8. Initialize Databroker & Lua
    auto* dbPtr = ctx.coordinator->data_broker();
    if (dbPtr == nullptr) {
      throw std::logic_error("Coordinator did not provide a DataBroker instance");
    }
    auto& db = *dbPtr;
    // Alias signals via lua
    {
      bool aliasing_failure = false;
      // Read cloe.alias_signals
      sol::object signal_aliases = cloe::luat_cloe_engine_initial_input(ctx.lua)["signal_aliases"];
      auto type = signal_aliases.get_type();
      switch (type) {
        // cloe.alias_signals: expected is a list (i.e. table) of 2-tuple each strings
        case sol::type::table: {
          sol::table alias_signals = signal_aliases.as<sol::table>();
          auto tbl_size = std::distance(alias_signals.begin(), alias_signals.end());
          //for (auto& kv : alias_signals)
          for (int i = 0; i < tbl_size; i++) {
            //sol::object value = kv.second;
            sol::object value = alias_signals[i + 1];
            sol::type type = value.get_type();
            switch (type) {
              // cloe.alias_signals[i]: expected is a 2-tuple (i.e. table) each strings
              case sol::type::table: {
                sol::table alias_tuple = value.as<sol::table>();
                auto tbl_size = std::distance(alias_tuple.begin(), alias_tuple.end());
                if (tbl_size != 2) {
                  // clang-format off
                  logger()->error(
                      "One or more entries in 'cloe.alias_signals' does not consist out of a 2-tuple. "
                      "Expected are entries in this format { \"regex\" , \"short-name\" }"
                      );
                  // clang-format on
                  aliasing_failure = true;
                  continue;
                }

                sol::object value;
                sol::type type;
                std::string old_name;
                std::string alias_name;
                value = alias_tuple[1];
                type = value.get_type();
                if (sol::type::string != type) {
                  // clang-format off
                  logger()->error(
                      "One or more parts in a tuple in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                      "Expected are entries in this format { \"regex\" , \"short-name\" }",
                      static_cast<int>(type));
                  // clang-format on
                  aliasing_failure = true;
                } else {
                  old_name = value.as<std::string>();
                }

                value = alias_tuple[2];
                type = value.get_type();
                if (sol::type::string != type) {
                  // clang-format off
                  logger()->error(
                      "One or more parts in a tuple in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                      "Expected are entries in this format { \"regex\" , \"short-name\" }",
                      static_cast<int>(type));
                  // clang-format on
                  aliasing_failure = true;
                } else {
                  alias_name = value.as<std::string>();
                }
                try {
                  db.alias(old_name, alias_name);
                  // clang-format off
                  logger()->info(
                      "Aliasing signal '{}' as '{}'.",
                      old_name, alias_name);
                  // clang-format on
                } catch (const std::logic_error& ex) {
                  // clang-format off
                  logger()->error(
                      "Aliasing signal specifier '{}' as '{}' failed with this error: {}",
                      old_name, alias_name, ex.what());
                  // clang-format on
                  aliasing_failure = true;
                } catch (...) {
                  // clang-format off
                  logger()->error(
                      "Aliasing signal specifier '{}' as '{}' failed.",
                      old_name, alias_name);
                  // clang-format on
                  aliasing_failure = true;
                }
              } break;
              // cloe.alias_signals[i]: is not a table
              default: {
                // clang-format off
                logger()->error(
                    "One or more entries in 'cloe.alias_signals' has an unexpected datatype '{}'. "
                    "Expected are entries in this format { \"regex\" , \"short-name\" }",
                    static_cast<int>(type));
                // clang-format on
                aliasing_failure = true;
              } break;
            }
          }

        } break;
        case sol::type::none:
        case sol::type::lua_nil: {
          // not defined -> nop
        } break;
        default: {
          // clang-format off
          logger()->error(
              "Expected symbol 'cloe.alias_signals' has unexpected datatype '{}'. "
              "Expected is a list of 2-tuples in this format { \"regex\" , \"short-name\" }",
              static_cast<int>(type));
          // clang-format on
          aliasing_failure = true;
        } break;
      }
      if (aliasing_failure) {
        throw cloe::ModelError("Aliasing signals failed with above error. Aborting.");
      }
    }

    // Inject requested signals into lua
    {
      auto& signals = db.signals();
      bool binding_failure = false;
      // Read cloe.require_signals
      sol::object value = cloe::luat_cloe_engine_initial_input(ctx.lua)["signal_requires"];
      auto type = value.get_type();
      switch (type) {
        // cloe.require_signals expected is a list (i.e. table) of strings
        case sol::type::table: {
          sol::table require_signals = value.as<sol::table>();
          auto tbl_size = std::distance(require_signals.begin(), require_signals.end());

          for (int i = 0; i < tbl_size; i++) {
            sol::object value = require_signals[i + 1];

            sol::type type = value.get_type();
            if (type != sol::type::string) {
              logger()->warn(
                  "One entry of cloe.require_signals has a wrong data type: '{}'. "
                  "Expected is a list of strings.",
                  static_cast<int>(type));
              binding_failure = true;
              continue;
            }
            std::string signal_name = value.as<std::string>();

            // virtually bind signal 'signal_name' to lua
            auto iter = db[signal_name];
            if (iter != signals.end()) {
              try {
                db.bind_signal(signal_name);
                logger()->info("Binding signal '{}' as '{}'.", signal_name, signal_name);
              } catch (const std::logic_error& ex) {
                logger()->error("Binding signal '{}' failed with error: {}", signal_name,
                                ex.what());
              }
            } else {
              logger()->warn("Requested signal '{}' does not exist in DataBroker.", signal_name);
              binding_failure = true;
            }
          }
          // actually bind all virtually bound signals to lua
          db.bind("signals", cloe::luat_cloe_engine(ctx.lua));
        } break;
        case sol::type::none:
        case sol::type::lua_nil: {
          logger()->warn(
              "Expected symbol 'cloe.require_signals' appears to be undefined. "
              "Expected is a list of string.");
        } break;
        default: {
          logger()->error(
              "Expected symbol 'cloe.require_signals' has unexpected datatype '{}'. "
              "Expected is a list of string.",
              static_cast<int>(type));
          binding_failure = true;
        } break;
      }
      if (binding_failure) {
        throw cloe::ModelError("Binding signals to Lua failed with above error. Aborting.");
      }
    }
  }
  ctx.progress.init_end();
  ctx.server->refresh_buffer_start_stream();
  logger()->info("Simulation initialization complete.");
  if (ctx.probe_simulation) {
    return PROBE;
  }
  return START;
}

}  // namespace engine
