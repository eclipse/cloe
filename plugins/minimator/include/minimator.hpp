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

#include <cloe/core/fable.hpp>  // for Confable, Schema, ...

namespace minimator {

struct ObjectPosition : public cloe::Confable {
  double x{0.0};
  double y{0.0};
  double z{0.0};

 public:
  CONFABLE_SCHEMA(ObjectPosition) {
    return cloe::Schema{
        {"x", cloe::Schema(&x, "Object position x axis")},
        {"y", cloe::Schema(&y, "Object position y axis")},
        {"z", cloe::Schema(&z, "Object position z axis")},
    };
  }
};

struct ObjectConfig : public cloe::Confable {
  double velocity{0.0};
  ObjectPosition position;

 public:
  CONFABLE_SCHEMA(ObjectConfig) {
    return cloe::Schema{
        {"velocity", cloe::Schema(&velocity, "Object longitudinal velocity")},
        {"position", cloe::Schema(&position, "Object position coordinates (x,y,z)")},
    };
  }
};

struct ObjectSensorConfig : public cloe::Confable {
  std::vector<ObjectConfig> objects;

 public:
  CONFABLE_SCHEMA(ObjectSensorConfig) {
    // clang-format off
    return cloe::Schema{
        {"objects", cloe::Schema(&objects, "Array of object configuration relative to ego vehicle")},
    };
    // clang-format on
  }
};

struct EgoSensorConfig : public cloe::Confable {
  ObjectConfig ego_object;

 public:
  CONFABLE_SCHEMA(EgoSensorConfig) {
    return cloe::Schema{
        {"ego_object", cloe::Schema(&ego_object, "Ego object configuration in world coordinates")},
    };
  }
};

struct SensorMockupConfig : public cloe::Confable {
  EgoSensorConfig ego_sensor_mockup;
  ObjectSensorConfig object_sensor_mockup;

 public:
  CONFABLE_SCHEMA(SensorMockupConfig) {
    // clang-format off
    return cloe::Schema{
        {"ego_sensor_mockup", cloe::Schema(&ego_sensor_mockup, "Ego sensor mockup configuration")},
        {"object_sensor_mockup", cloe::Schema(&object_sensor_mockup, "Object sensor mockup configuration")},
    };
    // clang-format on
  }
};

/**
 * MinimatorConfiguration is what we use to configure `Minimator` from some
 * JSON input.
 *
 * The Cloe runtime takes care of reading the configuration from the stack
 * file and passing it to the `MinimatorFactory`, which can then pass it
 * to `Minimator` during construction.
 *
 * So, the input will be deserialized from `/simulators/N/args`, where `N` is
 * some entry in the `simulators` object:
 *
 *     {
 *       "version": "4",
 *       "simulators": [
 *         {
 *           "binding": "minimator"
 *           "args": {
 *             "vehicles": [
 *               "ego1",
 *               "ego2"
 *             ]
 *           }
 *         }
 *       ]
 *     }
 *
 * Since our minimalistic simulator doesn't do much yet, our configuration
 * is quite simple: a number of names which will each become a vehicle.
 */

struct MinimatorConfiguration : public cloe::Confable {
  std::map<std::string, SensorMockupConfig> vehicles;
  // The `CONFABLE_SCHEMA` is simple enough and is the recommended way to
  // augment a class that inherits from `Confable` to expose `from_conf`,
  // `from_json`, and `to_json` methods. The `Schema` type is a sort of
  // polymorphic type that automatically derives a JSON schema from a set of
  // pointers. This schema is used to provide serialization and deserialization.
  //
  // \see fable/confable.hpp
  // \see fable/schema.hpp
  //
 public:
  CONFABLE_SCHEMA(MinimatorConfiguration) {
    // For us, each `Schema` describing a `Confable` will start with an
    // initializer list of pairs: this describes a JSON object. Each property in
    // this object may be another object or another primitive JSON type.
    // In this configuration, we want to deserialize into a vector of strings.
    //
    // `Schema` contains some magic to make it "easy" for you to use.
    // The following eventually boils down to:
    //
    //     fable::schema::Struct{
    //        {
    //          "vehicles",
    //          fable::schema::map<std::string, minimator::SensorMockupConfig>(
    //            &vehicles,
    //            "list of vehicle names to make available"
    //          )
    //        }
    //     }
    //
    // You can hopefully see why `Schema` contains the magic it contains.
    return cloe::Schema{
        {"vehicles", cloe::Schema(&vehicles, "list of vehicle names to make available")},
    };
  }
};

}
