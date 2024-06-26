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
 * \file simulation_machine.hpp
 *
 * This file provides the simulation state machine.
 *
 * The following flow diagram shows how the states of a simulation are
 * traversed in a typical simulation. The nominal flow is rendered in solid
 * lines, while irregular situations are rendered in dashed lines.
 *
 *                         ┌──────────────────────┐
 *           +------------ │       Connect        │
 *           |             └──────────────────────┘
 *           |                        │
 *           |                        ▼
 *           |             ┌──────────────────────┐
 *           +---...       │        Start         │ <-------------------------+
 *           |             └──────────────────────┘                           |
 *           |                        │                                       |
 *           |                        ▼                                       |
 *           |             ┌──────────────────────┐          +-----------+    |
 *           +---...       │      StepBegin       │ ◀──┐<--- |   Resume  |    |
 *           |             └──────────────────────┘    │     +-----------+    |
 *           |                        │                │           ^          |
 *           |                        ▼                │           |          |
 *           |             ┌──────────────────────┐    │           |          |
 *           +---...       │    StepSimulators    │    │           |          |
 *           |             └──────────────────────┘    │           |          |
 *           |                        │                │           |          |
 *           |                        ▼                │           |          |
 *           |             ┌──────────────────────┐    │           |          |
 *           +---...       │    StepControllers   │    │           |          |
 *           |             └──────────────────────┘    │           |          |
 *           |                        │                │           |          |
 *           v                        ▼                │           |          |
 *     +-----------+       ┌──────────────────────┐    │     +-----------+    |
 *     |   Abort   |       │       StepEnd        │ ───┘---> |   Pause   |    |
 *     +-----------+       └──────────────────────┘          +-----------+    |
 *         |    |                     │                         |     ^       |
 *         |    |             failure │ success                 |     |       |
 *         |    |                     ▼                         +-----+       |
 *         |    |          ┌──────────────────────┐          +-----------+    |
 *         |    +--------> │        Stop          │ -------> |   Reset   | ---+
 *         |               └──────────────────────┘          +-----------+
 *         |                          │
 *         |                          ▼
 *         |               ┌──────────────────────┐
 *         +-------------> │      Disconnect      │
 *                         └──────────────────────┘
 *
 * Note that not all possible transitions or states are presented in the above
 * diagram; for example, it is possible to go into the Abort state from almost
 * any other state. Neither can one see the constraints that apply to the above
 * transitions; for example, after Abort, the state machine may go into the
 * Stop state, but then will in every case go into the Disconnect state and
 * never into the Reset state.
 */

#pragma once

#include <future>  // for future<>, async

#include <cloe/core/abort.hpp>  // for AsyncAbort

#include "simulation_context.hpp"     // for SimulationContext
#include "utility/state_machine.hpp"  // for State, StateMachine

namespace engine {

/**
 * The SimulationMachine is statemachine with the given set of states and
 * simulation context.
 *
 * The state transitions are given by the states themselves and are not
 * stored in the simulation machine itself.
 *
 * The entry-point for this simulation machine is the run() method.
 *
 * If you want to modify the simulation flow, you need to do this with
 * the simulation context and by adding a new transition from the desired
 * state. You may need to add a new state, which you can do in this file
 * by defining it and then registering it in the SimulationMachine constructor.
 */
class SimulationMachine
    : private StateMachine<State<SimulationMachine, SimulationContext>, SimulationContext> {
  using SimulationState = State<SimulationMachine, SimulationContext>;

 public:
  SimulationMachine() {
    register_states({
        new Connect{this},
        new Start{this},
        new Probe{this},
        new StepBegin{this},
        new StepSimulators{this},
        new StepControllers{this},
        new StepEnd{this},
        new Pause{this},
        new Resume{this},
        new Success{this},
        new Fail{this},
        new Abort{this},
        new Stop{this},
        new Reset{this},
        new KeepAlive{this},
        new Disconnect{this},
    });
  }

  /**
   * This is the main entry-point of the simulation.
   *
   * This should be used even if you have a shortened simulation
   * flow, like CONNECT -> PROBING -> DISCONNECT.
   */
  void run(SimulationContext& ctx) { run_machine(CONNECT, ctx); }

  /**
   * Starting with the initial state, keep running states until the
   * sentinel state (nullptr) has been reached.
   */
  void run_machine(StateId initial, SimulationContext& ctx) {
    StateId id = initial;
    std::optional<StateId> interrupt;

    // Keep processing states as long as they are coming either from
    // an interrupt or from normal execution.
    while ((interrupt = pop_interrupt()) || id != nullptr) {
      try {
        // Handle interrupts that have been inserted via push_interrupt.
        // Only one interrupt is stored.
        //
        // If one interrupt follows another, the handler is responsible
        // for restoring nominal flow after all is done.
        if (interrupt) {
          id = handle_interrupt(id, *interrupt, ctx);
          continue;
        }

        if (ctx.config.engine.watchdog_mode == cloe::WatchdogMode::Off) {
          // Run state in this thread synchronously.
          id = run_state(id, ctx);
          continue;
        }

        id = run_state_async(id, ctx);
      } catch (cloe::AsyncAbort&) {
        this->push_interrupt(ABORT);
      } catch (cloe::ModelReset& e) {
        logger()->error("Unhandled reset request in {} state: {}", id, e.what());
        this->push_interrupt(RESET);
      } catch (cloe::ModelStop& e) {
        logger()->error("Unhandled stop request in {} state: {}", id, e.what());
        this->push_interrupt(STOP);
      } catch (cloe::ModelAbort& e) {
        logger()->error("Unhandled abort request in {} state: {}", id, e.what());
        this->push_interrupt(ABORT);
      } catch (cloe::ModelError& e) {
        logger()->error("Unhandled model error in {} state: {}", id, e.what());
        this->push_interrupt(ABORT);
      } catch (std::exception& e) {
        logger()->critical("Fatal error in {} state: {}", id, e.what());
        throw;
      }
    }
  }

  /**
   * Run state in a separate thread asynchronously and abort if
   * watchdog_timeout is exceeded.
   *
   * See configuration: stack.hpp
   * See documentation: doc/reference/watchdog.rst
   */
  StateId run_state_async(StateId id, SimulationContext& ctx) {
    std::chrono::milliseconds timeout = ctx.config.engine.watchdog_default_timeout;
    if (ctx.config.engine.watchdog_state_timeouts.count(id)) {
      auto maybe = ctx.config.engine.watchdog_state_timeouts[id];
      if (maybe) {
        timeout = *maybe;
      }
    }
    auto interval = timeout.count() > 0 ? timeout : ctx.config.engine.polling_interval;

    // Launch state
    std::future<StateId> f =
        std::async(std::launch::async, [this, id, &ctx]() { return run_state(id, ctx); });

    std::future_status status;
    for (;;) {
      status = f.wait_for(interval);
      if (status == std::future_status::ready) {
        return f.get();
      } else if (status == std::future_status::deferred) {
        if (timeout.count() > 0) {
          logger()->warn("Watchdog waiting on deferred execution.");
        }
      } else if (status == std::future_status::timeout) {
        if (timeout.count() > 0) {
          logger()->critical("Watchdog timeout of {} ms exceeded for state: {}", timeout.count(),
                             id);

          if (ctx.config.engine.watchdog_mode == cloe::WatchdogMode::Abort) {
            logger()->critical("Aborting simulation... this might take a while...");
            return ABORT;
          } else if (ctx.config.engine.watchdog_mode == cloe::WatchdogMode::Kill) {
            logger()->critical("Killing program... this is going to be messy...");
            std::abort();
          }
        }
      }
    }
  }

  // Asynchronous Actions:
  void pause() { this->push_interrupt(PAUSE); }
  void resume() { this->push_interrupt(RESUME); }
  void stop() { this->push_interrupt(STOP); }
  void succeed() { this->push_interrupt(SUCCESS); }
  void fail() { this->push_interrupt(FAIL); }
  void reset() { this->push_interrupt(RESET); }
  void abort() { this->push_interrupt(ABORT); }

  StateId handle_interrupt(StateId nominal, StateId interrupt, SimulationContext& ctx) override {
    logger()->debug("Handle interrupt: {}", interrupt);
    // We don't necessarily actually go directly to each desired state. The
    // states PAUSE and RESUME are prime examples; they should be entered and
    // exited from at pre-defined points.
    if (interrupt == PAUSE) {
      ctx.pause_execution = true;
    } else if (interrupt == RESUME) {
      ctx.pause_execution = false;
    } else {
      // All other interrupts will lead directly to the end of the
      // simulation.
      return this->run_state(interrupt, ctx);
    }
    return nominal;
  }

  friend void to_json(cloe::Json& j, const SimulationMachine& m) {
    j = cloe::Json{
        {"states", m.states()},
    };
  }

#define DEFINE_STATE(Id, S) DEFINE_STATE_STRUCT(SimulationMachine, SimulationContext, Id, S)
 public:
  DEFINE_STATE(CONNECT, Connect);
  DEFINE_STATE(PROBE, Probe);
  DEFINE_STATE(START, Start);
  DEFINE_STATE(STEP_BEGIN, StepBegin);
  DEFINE_STATE(STEP_SIMULATORS, StepSimulators);
  DEFINE_STATE(STEP_CONTROLLERS, StepControllers);
  DEFINE_STATE(STEP_END, StepEnd);
  DEFINE_STATE(PAUSE, Pause);
  DEFINE_STATE(RESUME, Resume);
  DEFINE_STATE(SUCCESS, Success);
  DEFINE_STATE(FAIL, Fail);
  DEFINE_STATE(ABORT, Abort);
  DEFINE_STATE(STOP, Stop);
  DEFINE_STATE(RESET, Reset);
  DEFINE_STATE(KEEP_ALIVE, KeepAlive);
  DEFINE_STATE(DISCONNECT, Disconnect);
#undef DEFINE_STATE
};

}  // namespace engine
