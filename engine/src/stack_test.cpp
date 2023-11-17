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
