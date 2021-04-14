/*
 * Copyright 2021 Robert Bosch GmbH
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
 * \file fable/schema/custom_test.cpp
 * \see  fable/schema/custom.hpp
 */

#include <string>  // for string
#include <vector>  // for vector<>

#include <gtest/gtest.h>  // for TEST

#include <fable/confable.hpp>       // for Confable
#include <fable/schema.hpp>         // for Schema, Variant, Conf, Json
#include <fable/schema/custom.hpp>  // for CustomDeserializer
#include <fable/utility/gtest.hpp>  // for assert_from_conf

namespace {

/**
 * MyCustomStruct models a command that can take either a string which it
 * passes to a shell to run, or an array, which it runs directly.
 *
 * It needs to make use of CustomDeserializer for this to work.
 * We dispense with the error checking that would be required in production
 * code, since this is just a unit test.
 */
struct MyCustomStruct : public fable::Confable {
  std::string executable;
  std::vector<std::string> args;

  CONFABLE_SCHEMA(MyCustomStruct) {
    // clang-format off
    using namespace fable;
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"command", Variant{
            "system command to execute",
            // Variant #1: system command to pass to a shell
            {
              CustomDeserializer(
                  make_prototype<std::string>(),
                  [this](CustomDeserializer*, const Conf& c) {
                    this->executable = "/bin/bash";
                    this->args = {"-c", c.get<std::string>()};
                  }
              ),
              // Variant #2: an array with the first element the command to run
              // and the rest of the items the arguments to the command.
              CustomDeserializer(
                  make_prototype<std::vector<std::string>>().min_items(1),
                  [this](CustomDeserializer*, const Conf& c) {
                    auto args = c.get<std::vector<std::string>>();
                    this->executable = args[0];
                    for (size_t i = 1; i < args.size(); ++i) {
                      this->args.push_back(args[i]);
                    }
                  }
              ),
            },
        }.require()}
    };
    // clang-format on
  }

  void to_json(fable::Json& j) const override {
    j = fable::Json{
        {"executable", executable},
        {"args", args},
    };
  }
};

}  // anonymous namespace

TEST(fable_schema_custom, schema) {
  MyCustomStruct tmp;
  fable::assert_schema_eq(tmp, R"({
    "type": "object",
    "properties": {
      "command": {
        "description": "system command to execute",
        "anyOf": [
          {
            "type": "string"
          },
          {
            "type": "array",
            "items": {
              "type": "string"
            },
            "minItems": 1
          }
        ]
      }
    },
    "required": ["command"],
    "additionalProperties": false
  })");
}

TEST(fable_schema_custom, from_conf) {
  MyCustomStruct tmp;

  fable::assert_from_conf(tmp, R"({
    "command": "echo 'Hello World'"
  })");
  ASSERT_EQ(tmp.executable, "/bin/bash");

  fable::assert_from_conf(tmp, R"({
    "command": ["echo", "Hello World!"]
  })");
  ASSERT_EQ(tmp.executable, "echo");

  fable::assert_invalidate(tmp, R"({
    "command": []
  })");
  ASSERT_EQ(tmp.executable, "echo");
}

TEST(fable_schema_custom, to_json) {
  MyCustomStruct tmp;
  tmp.executable = "echo";
  tmp.args = {"Hello World"};
  fable::assert_to_json(tmp, R"({
    "executable": "echo",
    "args": ["Hello World"]
  })");
}
