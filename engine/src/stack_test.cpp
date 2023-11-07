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
 * \file stack_test.cpp
 * \see  stack.hpp
 * \see  stack.cpp
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include <cloe/component.hpp>                // for DEFINE_COMPONENT_FACTORY
#include <cloe/component/ego_sensor.hpp>     // for EgoSensor
#include <cloe/component/object_sensor.hpp>  // for ObjectSensor
#include <cloe/core.hpp>                     // for Json
#include <fable/utility/gtest.hpp>           // for assert_from_conf
#include "stack.hpp"                         // for Stack
using namespace cloe;                        // NOLINT(build/namespaces)

TEST(cloe_stack, serialization_of_empty_stack) {
  Stack s;

  fable::assert_from_conf(s, R"({
    "version": "4.1"
  })");

  Json expect = R"({
    "engine": {
      "keep_alive": false,
      "hooks": {
        "post_disconnect": [],
        "pre_connect": []
      },
      "ignore": [],
      "output": {
        "clobber": true,
        "path": "${CLOE_SIMULATION_UUID}",
        "files": {
          "config": "config.json",
          "result": "result.json",
          "signals": "signals.json",
          "triggers": "triggers.json"
        }
      },
      "registry_path": "${XDG_DATA_HOME-${HOME}/.local/share}/cloe/registry",
      "plugin_path": [],
      "plugins": {
        "allow_clobber": true,
        "ignore_failure": false,
        "ignore_missing": false
      },
      "polling_interval": 100,
      "security": {
        "enable_command_action": false,
        "enable_include_section": true,
        "enable_hooks_section": true,
        "max_include_depth": 64
      },
      "triggers": {
        "ignore_source": false
      },
      "watchdog": {
        "mode": "off",
        "default_timeout": 90000,
        "state_timeouts": {
          "CONNECT": 300000,
          "ABORT": 90000,
          "STOP": 300000,
          "DISCONNECT": 600000
        }
      }
    },
    "controllers": [],
    "defaults": {
      "controllers": [],
      "components": [],
      "simulators": []
    },
    "logging": [],
    "plugins": [],
    "server": {
      "api_prefix": "/api",
      "listen": true,
      "listen_address": "127.0.0.1",
      "listen_port": 8080,
      "listen_threads": 10,
      "static_prefix": ""
    },
    "simulation": {
      "model_step_width": 20000000,
      "abort_on_controller_failure": true,
      "controller_retry_limit": 1000,
      "controller_retry_sleep": 1
    },
    "simulators": [],
    "triggers": [],
    "vehicles": [],
    "version": "4.1"
  })"_json;

  ASSERT_EQ(s.to_json().dump(2), expect.dump(2));
}

TEST(cloe_stack, serialization_with_logging) {
  Stack s;

  assert_from_conf(s, R"({
    "version": "4.1",
    "defaults": {
      "simulators": [
        { "binding": "vtd", "args": { "label_vehicle": "symbol" } }
      ]
    },
    "logging": [
      { "name": "*", "level": "info" }
    ]
  })");

  Json expect = R"({
    "engine": {
      "keep_alive": false,
      "hooks": {
        "post_disconnect": [],
        "pre_connect": []
      },
      "ignore": [],
      "output": {
        "clobber": true,
        "path": "${CLOE_SIMULATION_UUID}",
        "files": {
          "config": "config.json",
          "result": "result.json",
          "signals": "signals.json",
          "triggers": "triggers.json"
        }
      },
      "registry_path": "${XDG_DATA_HOME-${HOME}/.local/share}/cloe/registry",
      "plugin_path": [],
      "plugins": {
        "allow_clobber": true,
        "ignore_failure": false,
        "ignore_missing": false
      },
      "polling_interval": 100,
      "security": {
        "enable_command_action": false,
        "enable_include_section": true,
        "enable_hooks_section": true,
        "max_include_depth": 64
      },
      "triggers": {
        "ignore_source": false
      },
      "watchdog": {
        "mode": "off",
        "default_timeout": 90000,
        "state_timeouts": {
          "CONNECT": 300000,
          "ABORT": 90000,
          "STOP": 300000,
          "DISCONNECT": 600000
        }
      }
    },
    "controllers": [],
    "defaults": {
      "controllers": [],
      "components": [],
      "simulators": [
        { "binding": "vtd", "args": { "label_vehicle": "symbol" } }
      ]
    },
    "logging": [
      { "name": "*", "level": "info" }
    ],
    "plugins": [],
    "server": {
      "api_prefix": "/api",
      "listen": true,
      "listen_address": "127.0.0.1",
      "listen_port": 8080,
      "listen_threads": 10,
      "static_prefix": ""
    },
    "simulators": [],
    "simulation": {
      "model_step_width": 20000000,
      "abort_on_controller_failure": true,
      "controller_retry_limit": 1000,
      "controller_retry_sleep": 1
    },
    "triggers": [],
    "vehicles": [],
    "version": "4.1"
  })"_json;

  ASSERT_NO_THROW(Json{s.logging[0]}.dump());
  ASSERT_EQ(s.to_json().dump(2), expect.dump(2));

  Conf c{expect};
  ASSERT_TRUE(c.has_pointer("/engine/registry_path"));
}

struct DummySensorConf : public Confable {
  uint64_t freq;

  CONFABLE_SCHEMA(DummySensorConf) {
    return Schema{
        {"freq", Schema(&freq, "some frequency")},
    };
  }
};

class DummySensor : public NopObjectSensor {
 public:
  DummySensor(const std::string& name, const DummySensorConf& conf,
              std::shared_ptr<ObjectSensor> obs)
      : NopObjectSensor(), config_(conf), sensor_(obs) {}

  virtual ~DummySensor() noexcept = default;

  uint64_t get_freq() const { return config_.freq; }

 private:
  DummySensorConf config_;
  std::shared_ptr<ObjectSensor> sensor_;
};

DEFINE_COMPONENT_FACTORY(DummySensorFactory, DummySensorConf, "dummy_object_sensor",
                         "test component config")

DEFINE_COMPONENT_FACTORY_MAKE(DummySensorFactory, DummySensor, cloe::ObjectSensor)

TEST(cloe_stack, deserialization_of_component) {
  // Create a sensor component from the given configuration.
  std::shared_ptr<DummySensorFactory> cf = std::make_shared<DummySensorFactory>();
  ComponentConf cc = ComponentConf("dummy_sensor", cf);
  fable::assert_from_conf(cc, R"({
    "binding": "dummy_sensor",
    "name": "my_dummy_sensor",
    "from": "some_obj_sensor",
    "args" : {
      "freq" : 9
    }
  })");

  // In production code, "some_obj_sensor" would be fetched from a list of all
  // available sensors. Skip this step here.
  std::vector<std::shared_ptr<cloe::Component>> from = {std::shared_ptr<cloe::NopObjectSensor>()};
  auto d = std::dynamic_pointer_cast<DummySensor>(
      std::shared_ptr<cloe::Component>(std::move(cf->make(cc.args, from))));
  ASSERT_EQ(d->get_freq(), 9);
}

class FusionSensor : public NopObjectSensor {
 public:
  FusionSensor(const std::string& name, const DummySensorConf& conf,
               std::vector<std::shared_ptr<ObjectSensor>> obj_sensors,
               std::shared_ptr<EgoSensor> ego_sensor)
      : NopObjectSensor(), config_(conf), obj_sensors_(obj_sensors), ego_sensor_(ego_sensor) {}

  virtual ~FusionSensor() noexcept = default;

  uint64_t get_freq() const { return config_.freq; }

 private:
  DummySensorConf config_;
  std::vector<std::shared_ptr<ObjectSensor>> obj_sensors_;
  std::shared_ptr<EgoSensor> ego_sensor_;
};

DEFINE_COMPONENT_FACTORY(FusionSensorFactory, DummySensorConf, "fusion_object_sensor",
                         "test component config")

std::unique_ptr<::cloe::Component> FusionSensorFactory::make(
    const ::cloe::Conf& c, std::vector<std::shared_ptr<cloe::Component>> comp_src) const {
  decltype(config_) conf{config_};
  if (!c->is_null()) {
    conf.from_conf(c);
  }
  std::vector<std::shared_ptr<ObjectSensor>> obj_sensors;
  std::vector<std::shared_ptr<EgoSensor>> ego_sensors;
  for (auto& comp : comp_src) {
    auto obj_s = std::dynamic_pointer_cast<ObjectSensor>(comp);
    if (obj_s != nullptr) {
      obj_sensors.push_back(obj_s);
      continue;
    }
    auto ego_s = std::dynamic_pointer_cast<EgoSensor>(comp);
    if (ego_s != nullptr) {
      ego_sensors.push_back(ego_s);
      continue;
    }
    throw Error("FusionSensorFactory: Source component type not supported: from {}", comp->name());
  }
  if (ego_sensors.size() != 1) {
    throw Error("FusionSensorFactory: {}: Require exactly one ego sensor.", this->name());
  }
  return std::make_unique<FusionSensor>(this->name(), conf, obj_sensors, ego_sensors.front());
}

TEST(cloe_stack, deserialization_of_fusion_component) {
  // Create a sensor component from the given configuration.
  std::shared_ptr<FusionSensorFactory> cf = std::make_shared<FusionSensorFactory>();
  ComponentConf cc = ComponentConf("fusion_object_sensor", cf);
  fable::assert_from_conf(cc, R"({
    "binding": "fusion_object_sensor",
    "name": "my_fusion_sensor",
    "from": [
      "ego_sensor0",
      "obj_sensor1",
      "obj_sensor2"
    ],
    "args" : {
      "freq" : 77
    }
  })");

  // In production code, a component list containing "ego_sensor0", ... would
  // be generated. Skip this step here.
  std::vector<std::shared_ptr<cloe::Component>> sensor_subset = {
      std::make_shared<cloe::NopEgoSensor>(), std::make_shared<cloe::NopObjectSensor>(),
      std::make_shared<cloe::NopObjectSensor>()};
  auto f = std::dynamic_pointer_cast<FusionSensor>(
      std::shared_ptr<cloe::Component>(std::move(cf->make(cc.args, sensor_subset))));
  ASSERT_EQ(f->get_freq(), 77);
}
