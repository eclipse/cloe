/*
 * Copyright 2020 Robert Bosch GmbH
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
 * \file demo_stuck.cpp
 *
 * This file contains a demonstrator controller plugin that does not progress.
 * This controller can be used to test whether Cloe can detect controllers
 * that do not progress and that Cloe can successfully abort the simulation.
 */

#include <cassert>  // for assert

#include <cloe/component/object_sensor.hpp>  // for ObjectSensor
#include <cloe/controller.hpp>               // for Controller, ControllerFactory, ...
#include <cloe/plugin.hpp>                   // for EXPORT_CLOE_PLUGIN
#include <cloe/sync.hpp>                     // for Sync
#include <cloe/vehicle.hpp>                  // for Vehicle

namespace demo {

struct DemoStuckConf : public cloe::Confable {
  /**
   * How much progress is made after every call.
   */
  cloe::Duration progress_per_step{100'000};  // 100 us

  /**
   * At what time to get stuck.
   *
   * The runtime can still decide to stop progress.
   */
  cloe::Duration halt_progress_at{10'000'000'000};  // 10 s

  CONFABLE_SCHEMA(DemoStuckConf) {
    // clang-format off
    return cloe::Schema{
        {"progress_per_step", cloe::make_schema(&progress_per_step, "progress to make each step")},
        {"halt_progress_at", cloe::make_schema(&halt_progress_at, "time in ns at which to halt all progress")},
    };
    // clang-format on
  }
};

class DemoStuck : public cloe::Controller {
 public:
  DemoStuck(const std ::string& name, const DemoStuckConf& conf)
      : Controller(name), config_(conf) {}

  void reset() override {
    // Nothing to do here.
  }

  void abort() override {
    // Nothing to do here.
  }

  cloe::Duration process(const cloe::Sync& s) override {
    if (time_ < config_.halt_progress_at) {
      time_ += config_.progress_per_step;
    }
    logger()->trace("Progressing by {} to {} <= {}", cloe::to_string(config_.progress_per_step),
                    cloe::to_string(time_), cloe::to_string(s.time()));
    return time_;
  }

 private:
  cloe::Duration time_{0};
  DemoStuckConf config_;
};

DEFINE_CONTROLLER_FACTORY(DemoStuckFactory, DemoStuckConf, "demo_stuck",
                          "slowly progressing demo controller")
DEFINE_CONTROLLER_FACTORY_MAKE(DemoStuckFactory, DemoStuck)

}  // namespace demo

// Register factory as plugin entrypoint
EXPORT_CLOE_PLUGIN(demo::DemoStuckFactory)
