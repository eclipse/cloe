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
 * \file fable/schema/factory_test.cpp
 * \see  fable/schema/factory.hpp
 */

#include <string>  // for string

#include <gtest/gtest.h>  // for TEST, ...

#include <fable/confable.hpp>        // for Confable
#include <fable/schema.hpp>          // for Schema, Struct, ...
#include <fable/schema/factory.hpp>  // for Factory
#include <fable/utility.hpp>         // for pretty_print
#include <fable/utility/gtest.hpp>   // for assert_schema_eq, ...

namespace {

struct MyFactoryStruct : public fable::Confable {
  int number = -1;

  CONFABLE_SCHEMA(MyFactoryStruct) {
    // clang-format off
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {
          "number",
          Factory<int>(&number, "number choice", {
            {
              "prime", {
                Struct{
                  {"n", fable::make_prototype<uint8_t>().bounds_with(1,6, {}).require()},
                },
                [](const fable::Conf& c) -> int {
                  static const std::vector<int> primes{2, 3, 5, 7, 11, 13};
                  auto n = c.get<uint8_t>("n");
                  return primes[n-1];
                },
              },
            },
            {
              "nonprime", {
                Struct{},
                [](const fable::Conf& c) -> int {
                  return 4;
                },
              },
            },
          }),
        }
    };
    // clang-format on
  }
};

}  // anonymous namespace

TEST(fable_schema_factory, schema) {
  MyFactoryStruct tmp;

  fable::assert_schema_eq(tmp, R"({
    "additionalProperties": false,
    "properties": {
      "number": {
        "description": "number choice",
        "oneOf": [
          {
            "additionalProperties": false,
            "properties": {
              "args": {
                "additionalProperties": false,
                "properties": {},
                "type": "object"
              },
              "factory": {
                "const": "nonprime",
                "description": "name of factory"
              }
            },
            "required": [
              "factory"
            ],
            "type": "object"
          },
          {
            "additionalProperties": false,
            "properties": {
              "args": {
                "additionalProperties": false,
                "properties": {
                  "n": {
                    "maximum": 6,
                    "minimum": 1,
                    "type": "integer"
                  }
                },
                "required": [
                  "n"
                ],
                "type": "object"
              },
              "factory": {
                "const": "prime",
                "description": "name of factory"
              }
            },
            "required": [
              "factory"
            ],
            "type": "object"
          }
        ]
      }
    },
    "type": "object"
  })");
}

TEST(fable_schema_factory, validate) {
  MyFactoryStruct tmp;
  tmp.number = 0;

  fable::assert_validate(tmp, R"({
    "number": {
      "factory": "prime",
      "args": {
        "n": 1
      }
    }
  })");
  ASSERT_EQ(tmp.number, 0) << "validation should not modify tmp";

  fable::assert_validate(tmp, R"({
    "number": {
      "factory": "nonprime"
    }
  })");
  ASSERT_EQ(tmp.number, 0) << "validation should not modify tmp";
}

TEST(fable_schema_factory, to_json) {
  MyFactoryStruct tmp;
  tmp.number = 1;

  fable::assert_to_json(tmp, R"({"number": 1})");
}

TEST(fable_schema_factory, from_conf) {
  MyFactoryStruct tmp;
  fable::assert_from_conf(tmp, R"({
    "number": {
      "factory": "prime",
      "args": {
        "n": 1
      }
    }
  })");
  ASSERT_EQ(tmp.number, 2);

  fable::assert_from_conf(tmp, R"({
    "number": {
      "factory": "nonprime"
    }
  })");
  ASSERT_EQ(tmp.number, 4);
}

// ------------------------------------------------------------------------- //

namespace {

struct MyFactoryStructWithoutArgs : public fable::Confable {
  int number;

  CONFABLE_SCHEMA(MyFactoryStructWithoutArgs) {
    // clang-format off
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {
          "number",
          Factory<int>(&number, "number choice", {
            {
              "prime", {
                Struct{
                  {"n", fable::make_prototype<uint8_t>().bounds_with(1, 6, {}).require()},
                },
                [](const fable::Conf& c) -> int {
                  static const std::vector<int> primes{2, 3, 5, 7, 11, 13};
                  auto n = c.get<uint8_t>("n");
                  return primes[n-1];
                },
              },
            },
            {
              "nonprime", {
                Struct{},
                [](const fable::Conf& c) -> int {
                  return 4;
                },
              },
            },
          }).args_key(""),
        }
    };
    // clang-format on
  }
};

}  // anonymous namespace

TEST(fable_schema_factory, without_args_schema) {
  MyFactoryStructWithoutArgs tmp;

  fable::assert_schema_eq(tmp, R"({
    "additionalProperties": false,
    "properties": {
      "number": {
        "description": "number choice",
        "oneOf": [
          {
            "additionalProperties": false,
            "properties": {
              "factory": {
                "const": "nonprime",
                "description": "name of factory"
              }
            },
            "required": [
              "factory"
            ],
            "type": "object"
          },
          {
            "additionalProperties": false,
            "properties": {
              "factory": {
                "const": "prime",
                "description": "name of factory"
              },
              "n": {
                "maximum": 6,
                "minimum": 1,
                "type": "integer"
              }
            },
            "required": [
              "factory",
              "n"
            ],
            "type": "object"
          }
        ]
      }
    },
    "type": "object"
  })");
}

TEST(fable_schema_factory, without_args_validate) {
  MyFactoryStructWithoutArgs tmp;
  tmp.number = 0;

  fable::assert_validate(tmp, R"({
    "number": {
      "factory": "prime",
      "n": 1
    }
  })");
  ASSERT_EQ(tmp.number, 0) << "validation should not modify tmp";

  fable::assert_validate(tmp, R"({
    "number": {
      "factory": "nonprime"
    }
  })");
  ASSERT_EQ(tmp.number, 0) << "validation should not modify tmp";
}

TEST(fable_schema_factory, without_args_to_json) {
  MyFactoryStructWithoutArgs tmp;
  tmp.number = 1;
  fable::assert_to_json(tmp, R"({"number": 1})");
}

TEST(fable_schema_factory, without_args_from_conf) {
  MyFactoryStructWithoutArgs tmp;
  fable::assert_from_conf(tmp, R"({
    "number": {
      "factory": "prime",
      "n": 1
    }
  })");
  ASSERT_EQ(tmp.number, 2);

  fable::assert_from_conf(tmp, R"({
    "number": {
      "factory": "nonprime"
    }
  })");
  ASSERT_EQ(tmp.number, 4);
}
