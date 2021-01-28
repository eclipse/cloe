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

#include <cloe/component.hpp>                // for DEFINE_COMPONENT_FACTORY
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
    std::shared_ptr<cloe::Component> from = std::shared_ptr<cloe::Component>{nullptr};
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
    std::shared_ptr<cloe::Component> from = std::shared_ptr<cloe::Component>{nullptr};
    auto d = std::dynamic_pointer_cast<DummySensor>(std::shared_ptr<cloe::Component>(
        std::move(cf->make(vc.components.find("cloe::default_world_sensor")->second.args, from))));
    ASSERT_EQ(d->get_freq(), 10);
  }
}