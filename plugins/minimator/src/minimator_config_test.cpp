/*
 * Copyright 2023 Robert Bosch GmbH
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

#include <gtest/gtest.h>
#include <fable/utility/gtest.hpp>  // for assert_validate

#include <cloe/core/fable.hpp>  // for assert_validate
#include <minimator.hpp>

namespace minimator {

TEST(minimator, deserialize_object_position) {
  ObjectPosition pos;

  fable::assert_validate(pos, R"({
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
    })");
}

TEST(minimator, deserialize_object_config) {
  ObjectConfig obj_conf;

  fable::assert_validate(obj_conf, R"({
    "position": {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
    },
    "velocity" : 0.0
    })");
}

TEST(minimator, deserialize_object_sensor_config) {
  ObjectSensorConfig obj_sensor_conf;

  fable::assert_validate(obj_sensor_conf, R"({
        "objects": [
        {
            "velocity": 0.0,
            "position": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
            }
        }
        ]
    })");
}

TEST(minimator, deserialize_ego_sensor_config) {
  EgoSensorConfig ego_sensor_conf;

  fable::assert_validate(ego_sensor_conf, R"({
        "ego_object": {
        "velocity": 0.0,
        "position": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
        }
        }
    })");
}

TEST(minimator, deserialize_sensor_mockup_config) {
  SensorMockupConfig sensor_mockup_conf;

  fable::assert_validate(sensor_mockup_conf, R"({
        "ego_sensor_mockup": {
            "ego_object": {
            "velocity": 0.0,
            "position": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
            }
            }
        },
        "object_sensor_mockup": {
            "objects": [
            {
                "velocity": 0.0,
                "position": {
                "x": 0.0,
                "y": 0.0,
                "z": 0.0
                }
            }
            ]
        }
    })");
}

TEST(minimator, deserialize_minimator_config) {
  MinimatorConfiguration minimator_config;

  fable::assert_validate(minimator_config, R"({
        "vehicles": {
          "ego1": {
            "ego_sensor_mockup": {
                "ego_object": {
                "velocity": 0.0,
                "position": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                }
                }
            },
            "object_sensor_mockup": {
                "objects": [
                {
                    "velocity": 0.0,
                    "position": {
                    "x": 0.0,
                    "y": 0.0,
                    "z": 0.0
                    }
                }
                ]
            }
        }
        }
    })");
}

}  // namespace minimator
