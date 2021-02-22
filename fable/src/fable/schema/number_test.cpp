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
 * \file fable/schema/number_test.cpp
 * \see  fable/schema/number.hpp
 */

#include <iostream>   // for cout, endl
#include <string>     // for string
using namespace std;  // NOLINT(build/namespaces)

#include <gtest/gtest.h>

#include <fable/confable.hpp>       // for Confable
#include <fable/schema.hpp>         // for Number
#include <fable/utility/gtest.hpp>  // for assert_validate

struct MyNumberStruct : public fable::Confable {
  uint8_t number;
  CONFABLE_SCHEMA(MyNumberStruct) {
    using namespace fable::schema;  // NOLINT(build/namespace)
    return Struct{
        {"number", make_schema(&number, "special number").bounds(0, 7).whitelist(15)},
    };
  }
};

TEST(fable_schema_number, schema) {
  MyNumberStruct tmp;
  fable::assert_schema_eq(tmp, R"({
    "type": "object",
    "properties": {
      "number": {
        "description": "special number",
        "type": "integer",
        "minimum": 0,
        "maximum": 7,
        "whitelist": [
          15
        ]
      }
    },
    "additionalProperties": false
  })");
}

TEST(fable_schema_number, validate) {
  MyNumberStruct tmp;

  std::vector<uint8_t> ok{0, 1, 2, 3, 4, 5, 6, 7, 15};
  for (auto x : ok) {
    fable::assert_validate(tmp, fable::Json{
                                    {"number", x},
                                });
  };

  std::vector<uint8_t> wrong{8, 16, 32, 255};
  for (auto x : wrong) {
    fable::assert_invalidate(tmp, fable::Json{
                                      {"number", x},
                                  });
  }
}
