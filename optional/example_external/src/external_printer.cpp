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
 * \file external_printer.cpp
 */

#include <cassert>  // for assert

#include <cloe/component/object_sensor.hpp>  // for ObjectSensor
#include <cloe/controller.hpp>               // for Controller, ControllerFactory, ...
#include <cloe/models.hpp>                   // for CloeComponent
#include <cloe/plugin.hpp>                   // for EXPORT_CLOE_PLUGIN
#include <cloe/sync.hpp>                     // for Sync
#include <cloe/vehicle.hpp>                  // for Vehicle

namespace external {

struct ExternalPrinterConf : public cloe::Confable {
  CONFABLE_FRIENDS(ExternalPrinterConf)
};

class ExternalPrinter : public cloe::Controller {
 public:
  using Controller::Controller;

  void reset() override {
    // Nothing to do here.
  }

  void abort() override {
    // Nothing to do here.
  }

  cloe::Duration process(const cloe::Sync& sync) override {
    assert(veh_ != nullptr);
    auto log = this->logger();
    log->info("External Step {} @ {}", sync.step(), sync.time().count());
    if (veh_->has(cloe::CloeComponent::DEFAULT_WORLD_SENSOR)) {
      auto sensor = veh_->get<cloe::ObjectSensor>(cloe::CloeComponent::DEFAULT_WORLD_SENSOR);
      auto& objs = sensor->sensed_objects();
      log->info("  {} Objects", objs.size());
      for (auto o : objs) {
        log->info("    id={} pos=({:3f}, {:3f}, {:3f})", o->id, o->pose.translation()(0),
                  o->pose.translation()(1), o->pose.translation()(2));
      }
    }
    return sync.time();
  }
};

DEFINE_CONTROLLER_FACTORY(ExternalPrinterFactory, ExternalPrinterConf, "external_printer",
                          "print a lot of information")

std::unique_ptr<cloe::Controller> ExternalPrinterFactory::make(const cloe::Conf&) const {
  return std::make_unique<ExternalPrinter>(this->name());
}

}  // namespace external

// Register factory as plugin entrypoint
EXPORT_CLOE_PLUGIN(external::ExternalPrinterFactory)
