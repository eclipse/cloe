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
 * \file simulation_state_start.cpp
 */

#include <cloe/core/error.hpp>  // for ConcludedError, TriggerError
#include <fable/error.hpp>      // for SchemaError
#include <fable/utility.hpp>    // for pretty_print

#include "coordinator.hpp"         // for Coordinator::trigger_registrar
#include "simulation_context.hpp"  // for SimulationContext
#include "simulation_machine.hpp"  // for SimulationMachine

namespace engine {

size_t insert_triggers_from_config(SimulationContext& ctx) {
  auto r = ctx.coordinator->trigger_registrar(cloe::Source::FILESYSTEM);
  size_t count = 0;
  for (const auto& c : ctx.config.triggers) {
    if (!ctx.config.engine.triggers_ignore_source && source_is_transient(c.source)) {
      continue;
    }
    try {
      r->insert_trigger(c.conf());
      count++;
    } catch (fable::SchemaError& e) {
      ctx.logger()->error("Error inserting trigger: {}", e.what());
      std::stringstream s;
      fable::pretty_print(e, s);
      ctx.logger()->error("> Message:\n    {}", s.str());
      throw cloe::ConcludedError(e);
    } catch (cloe::TriggerError& e) {
      ctx.logger()->error("Error inserting trigger ({}): {}", e.what(), c.to_json().dump());
      throw cloe::ConcludedError(e);
    }
  }
  return count;
}

StateId SimulationMachine::Start::impl(SimulationContext& ctx) {
  logger()->info("Starting simulation...");

  // Begin execution progress
  ctx.progress.exec_begin();

  // Process initial trigger list
  insert_triggers_from_config(ctx);
  ctx.coordinator->process_pending_lua_triggers(ctx.sync);
  ctx.coordinator->process(ctx.sync);
  ctx.callback_start->trigger(ctx.sync);

  // Process initial context
  ctx.foreach_model([this, &ctx](cloe::Model& m, const char* type) {
    logger()->trace("Start {} {}", type, m.name());
    m.start(ctx.sync);
    return true;  // next model
  });
  ctx.sync.increment_step();

  // We can pause at the start of execution too.
  if (ctx.pause_execution) {
    return PAUSE;
  }

  return STEP_BEGIN;
}

}  // namespace engine
