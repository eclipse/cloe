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
 * \file simulation_actions.hpp
 *
 * This file defines the simple actions inherent to
 * the simulation machine itself.
 */

#pragma once

#include <cloe/trigger.hpp>             // for Trigger, Event, EventFactory, ...
#include <cloe/trigger/set_action.hpp>  // for DEFINE_SET_STATE_ACTION, SetDataActionFactory

#include "simulation_context.hpp"
#include "simulation_machine.hpp"
#include "simulation_statistics.hpp"
#include "simulation_sync.hpp"

namespace engine::actions {

DEFINE_SET_STATE_ACTION(Pause, "pause", "pause simulation", SimulationMachine, { ptr_->pause(); })

DEFINE_SET_STATE_ACTION(Resume, "resume", "resume paused simulation", SimulationMachine,
                        { ptr_->resume(); })

DEFINE_SET_STATE_ACTION(Stop, "stop", "stop simulation with neither success nor failure",
                        SimulationMachine, { ptr_->stop(); })

DEFINE_SET_STATE_ACTION(Succeed, "succeed", "stop simulation with success", SimulationMachine,
                        { ptr_->succeed(); })

DEFINE_SET_STATE_ACTION(Fail, "fail", "stop simulation with failure", SimulationMachine,
                        { ptr_->fail(); })

DEFINE_SET_STATE_ACTION(Reset, "reset", "attempt to reset simulation", SimulationMachine,
                        { ptr_->reset(); })

DEFINE_SET_STATE_ACTION(KeepAlive, "keep_alive", "keep simulation alive after termination",
                        SimulationContext, { ptr_->config.engine.keep_alive = true; })

DEFINE_SET_STATE_ACTION(ResetStatistics, "reset_statistics", "reset simulation statistics",
                        SimulationStatistics, { ptr_->reset(); })

DEFINE_SET_DATA_ACTION(RealtimeFactor, "realtime_factor", "modify the simulation speed",
                       SimulationSync, "factor", double, {
                         logger()->info("Setting target simulation speed: {}", value_);
                         ptr_->set_realtime_factor(value_);
                       })

}  // namespace engine::actions
