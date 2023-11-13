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

#include <string>
#include <map>
#include <vector>

#include <gtest/gtest.h>

#include <fable/confable.hpp>
#include <fable/schema.hpp>
#include <fable/utility/gtest.hpp>

namespace {

struct Vec3d: public fable::Confable {
  double x{0.0}; // NOLINT
  double y{0.0}; // NOLINT
  double z{0.0}; // NOLINT

 public:
  CONFABLE_SCHEMA(Vec3d) {
    return fable::Schema{
        {"x", fable::Schema(&x, "Object position x axis")},
        {"y", fable::Schema(&y, "Object position y axis")},
        {"z", fable::Schema(&z, "Object position z axis")},
    };
  }
};

struct Object : public fable::Confable {
  double velocity{0.0}; // NOLINT
  Vec3d position; // NOLINT

 public:
  CONFABLE_SCHEMA(Object) {
    return fable::Schema{
        {"velocity", fable::Schema(&velocity, "Object longitudinal velocity")},
        {"position", fable::Schema(&position, "Object position coordinates (x,y,z)")},
    };
  }
};

struct ObjectContainer : public fable::Confable {
  std::vector<Object> objects; // NOLINT

 public:
  CONFABLE_SCHEMA(ObjectContainer) {
    return fable::Schema{
        {"objects", fable::Schema(&objects, "")},
    };
  }
};

struct NamedObject : public fable::Confable {
  Object named_object; // NOLINT

 public:
  CONFABLE_SCHEMA(NamedObject) {
    return fable::Schema{
        {"named_object", fable::Schema(&named_object, "")},
    };
  }
};

struct NestedNamedObject : public fable::Confable {
  NamedObject ego_sensor_mockup; // NOLINT

 public:
  CONFABLE_SCHEMA(NestedNamedObject) {
    return fable::Schema{
        {"ego_sensor_mockup", fable::Schema(&ego_sensor_mockup, "Ego sensor mockup configuration")},
    };
  }
};

template <typename T>
struct MapOfSomething : public fable::Confable {
  std::map<std::string, T> values; // NOLINT

 public:
  CONFABLE_SCHEMA(MapOfSomething<T>) {
    return fable::Schema{
        {"values", fable::Schema(&values, "")},
    };
  }
};

} // anonymous namespace

TEST(fable_schema_map, validate_map_of_vec3d) {
  MapOfSomething<Vec3d> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": { "x": 1.0, "y": 2.0, "z": 3.0 },
      "b": { "x": 0.0, "y": 0.0, "z": 0.0 }
    }
  })");
}

TEST(fable_schema_map, validate_map_of_object) {
  MapOfSomething<Object> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": { "position": { "x": 1.0, "y": 2.0, "z": 3.0 }, "velocity": 0.0 },
      "b": { "position": { "x": 0.0, "y": 0.0, "z": 0.0 } },
      "c": { },
      "d": { "velocity": 1.0 }
    }
  })");
}

TEST(fable_schema_map, validate_map_of_objectcontainer) {
  MapOfSomething<ObjectContainer> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": { "objects": [{ "position": { "x": 1.0, "y": 2.0, "z": 3.0 }, "velocity": 0.0 }] },
      "b": { "objects": [{ "position": { "x": 0.0, "y": 0.0, "z": 0.0 } }] },
      "c": { "objects": [{ }] },
      "d": { "objects": [{ "velocity": 1.0 }] }
    }
  })");
}

TEST(fable_schema_map, validate_map_of_namedobject) {
  MapOfSomething<NamedObject> wrapper;

  fable::assert_validate(wrapper, R"({
    "values": {
      "a": { "named_object": { } }
    }
  })");
}

TEST(fable_schema_map, validate_map_of_fromconfable) {
  MapOfSomething<NestedNamedObject> minimator_config;

  fable::assert_validate(minimator_config, R"({
    "values": {
      "ego1": {
        "ego_sensor_mockup": {
          "named_object": {
            "velocity": 0.0,
            "position": {
              "x": 0.0,
              "y": 0.0,
              "z": 0.0
            }
          }
        }
      }
    }
  })");
}
