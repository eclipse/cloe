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
#include <map>
#include <string>
#include <vector>

#include <cloe/component.hpp>                // for DEFINE_COMPONENT_FACTORY
#include <cloe/component/ego_sensor.hpp>     // for EgoSensor
#include <cloe/component/object_sensor.hpp>  // for ObjectSensor
#include <cloe/core.hpp>                     // for Json
#include <fable/utility.hpp>                 // for pretty_print
#include "stack.hpp"                         // for Stack
using namespace cloe;                        // NOLINT(build/namespaces)

TEST(cloe_stack, serialization_of_empty_stack) {
  Stack s;

  Json input = R"({
    "version": "4"
  })"_json;
  s.from_conf(Conf{input});

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
    "version": "4"
  })"_json;

  ASSERT_EQ(s.to_json().dump(2), expect.dump(2));
}

TEST(cloe_stack, serialization_with_logging) {
  Stack s;

  Json input = R"({
    "version": "4",
    "defaults": {
      "simulators": [
        { "binding": "vtd", "args": { "label_vehicle": "symbol" } }
      ]
    },
    "logging": [
      { "name": "*", "level": "info" }
    ]
  })"_json;
  s.from_conf(Conf{input});

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
    "version": "4"
  })"_json;

  ASSERT_NO_THROW(Json{s.logging[0]}.dump());
  ASSERT_EQ(s.to_json().dump(2), expect.dump(2));

  Conf c{expect};
  ASSERT_TRUE(c.has_pointer("/engine/registry_path"));
}

struct DummySensorConf : public Confable {
  uint64_t freq;

  Schema schema_impl() override {
    return Schema{
        {"freq", Schema(&freq, "some frequency")},
    };
  }

  void to_json(Json& j) const override {
    j = Json{
        {"freq", freq},
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
  Json input = R"({        
    "binding": "dummy_sensor",
    "name": "my_dummy_sensor",
    "from": "cloe::default_world_sensor",
    "args" : {
      "freq" : 9
    }
  })"_json;

  {
    std::shared_ptr<DummySensorFactory> cf = std::make_shared<DummySensorFactory>();
    ComponentConf cc = ComponentConf("dummy_sensor", cf);
    // Create a sensor component from the given configuration.
    cc.from_conf(Conf{input});
    std::map<std::string, std::shared_ptr<cloe::Component>> vehicle_sensors = {
        {"cloe::default_world_sensor", std::shared_ptr<ObjectSensor>{nullptr}}};
    std::map<std::string, std::shared_ptr<cloe::Component>> from;
    for (const auto& kv : cc.from) {
      from[kv.first] = vehicle_sensors.find(kv.second)->second;
    }
    auto d = std::dynamic_pointer_cast<DummySensor>(
        std::shared_ptr<cloe::Component>(std::move(cf->make(cc.args, from))));
    ASSERT_EQ(d->get_freq(), 9);
  }

  input = R"({
    "name": "default_vehicle",
      "from": {
        "simulator": "test",
        "index": 0
      },
      "components" : {
        "cloe::default_world_sensor": {
          "binding": "dummy_sensor",
          "name": "my_dummy_sensor",
          "from": "cloe::default_world_sensor",
          "args" : {
            "freq" : 10
          }
        }
      }
  })"_json;

  {
    ComponentSchema cs;
    std::shared_ptr<DummySensorFactory> cf = std::make_shared<DummySensorFactory>();
    cs.add_factory("dummy_sensor", cf);
    VehicleConf vc{cs};
    // Create a sensor component from the given configuration.
    vc.from_conf(Conf{input});
    auto cc = vc.components.find("cloe::default_world_sensor")->second;
    std::map<std::string, std::shared_ptr<cloe::Component>> vehicle_sensors = {
        {"cloe::default_world_sensor", std::shared_ptr<ObjectSensor>{nullptr}}};
    std::map<std::string, std::shared_ptr<cloe::Component>> from;
    for (const auto& kv : cc.from) {
      from[kv.first] = vehicle_sensors.find(kv.second)->second;
    }
    auto d = std::dynamic_pointer_cast<DummySensor>(
        std::shared_ptr<cloe::Component>(std::move(cf->make(cc.args, from))));
    ASSERT_EQ(d->get_freq(), 10);
  }
}

class FusionSensor : public NopObjectSensor {
 public:
  FusionSensor(const std::string& name, const DummySensorConf& conf,
               std::shared_ptr<ObjectSensor> obs1, std::shared_ptr<ObjectSensor> obs2,
               std::shared_ptr<EgoSensor> egos)
      : NopObjectSensor(), config_(conf), obj_src_a_(obs1), obj_src_b_(obs2), ego_sensor_(egos) {}

  virtual ~FusionSensor() noexcept = default;

  uint64_t get_freq() const { return config_.freq; }

 private:
  DummySensorConf config_;
  std::shared_ptr<ObjectSensor> obj_src_a_;
  std::shared_ptr<ObjectSensor> obj_src_b_;
  std::shared_ptr<EgoSensor> ego_sensor_;
};

DEFINE_COMPONENT_FACTORY(FusionSensorFactory, DummySensorConf, "fusion_object_sensor",
                         "test component config")

std::unique_ptr<::cloe::Component> FusionSensorFactory::make(
    const ::cloe::Conf& c,
    const std::map<std::string, std::shared_ptr<cloe::Component>>& comp) const {
  decltype(config_) conf{config_};
  if (!c->is_null()) {
    conf.from_conf(c);
  }
  std::vector<std::string> comp_bindings{"object_sensor_1", "object_sensor_2", "ego_sensor"};
  for (const auto& cb : comp_bindings) {
    if (comp.find(cb) != comp.end()) {
      continue;
    }
    throw Error("FusionSensorFactory: Source component configuration not found: from {}", cb);
  }
  return std::make_unique<FusionSensor>(
      this->name(), conf,
      std::dynamic_pointer_cast<ObjectSensor>(comp.find(comp_bindings.at(0))->second),
      std::dynamic_pointer_cast<ObjectSensor>(comp.find(comp_bindings.at(1))->second),
      std::dynamic_pointer_cast<EgoSensor>(comp.find(comp_bindings.at(2))->second));
}

TEST(cloe_stack, deserialization_of_fusion_component) {
  Json input = R"({
      "binding": "fusion_sensor",
      "name": "dummy_fusion_sensor",
      "from": {
        "object_sensor_1": "left_corner_sensor",
        "object_sensor_2": "right_corner_sensor",
        "ego_sensor": "cloe::default_ego_sensor"
      },
      "args" : {
        "freq" : 11
      }
  })"_json;

  {
    std::shared_ptr<FusionSensorFactory> cf = std::make_shared<FusionSensorFactory>();
    ComponentConf cc = ComponentConf("fusion_sensor", cf);
    // Create a sensor component from the given configuration.
    cc.from_conf(Conf{input});
    std::map<std::string, std::shared_ptr<cloe::Component>> vehicle_sensors = {
        {"left_corner_sensor", std::shared_ptr<ObjectSensor>{nullptr}},
        {"right_corner_sensor", std::shared_ptr<ObjectSensor>{nullptr}},
        {"cloe::default_ego_sensor", std::shared_ptr<EgoSensor>{nullptr}}};
    std::map<std::string, std::shared_ptr<cloe::Component>> from;
    for (const auto& kv : cc.from) {
      from[kv.first] = vehicle_sensors.find(kv.second)->second;
    }
    auto f = std::dynamic_pointer_cast<FusionSensor>(
        std::shared_ptr<cloe::Component>(std::move(cf->make(cc.args, from))));
    ASSERT_EQ(f->get_freq(), 11);
  }
}