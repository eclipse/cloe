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

#include <fable/confable.hpp>           // for Confable
#include <fable/schema.hpp>             // for Number
#include <fable/utility/gtest.hpp>      // for assert_validate
#include <fable/utility/templates.hpp>  // for typeinfo

struct MyNumberStruct : public fable::Confable {
  uint8_t number = 0;
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

template <typename T>
void assert_validate_t(const fable::Schema& schema, const std::vector<T>& xs) {
  for (T x : xs) {
    std::cerr << "Expect ok   " << fable::typeinfo<T>::name << ":\t" << (+x) << std::endl;
    fable::assert_validate(schema, fable::Json(x));
  }
}

template <typename T>
void assert_invalidate_t(const fable::Schema& schema, const std::vector<T>& xs) {
  for (T x : xs) {
    std::cerr << "Expect fail " << fable::typeinfo<T>::name << ":\t" << (+x) << std::endl;
    fable::assert_invalidate(schema, fable::Json(x));
  }
}

TEST(fable_schema_number, validate_u64) {
  auto schema = fable::schema::Number<uint64_t>(nullptr, "");

  assert_validate_t<uint64_t>(schema, {0, 1, std::numeric_limits<uint64_t>::max()});
  assert_validate_t<int64_t>(schema, {0, 1, std::numeric_limits<int64_t>::max()});

  assert_invalidate_t<int8_t>(schema, {-1, -5, std::numeric_limits<int8_t>::min()});
  assert_invalidate_t<int64_t>(schema, {-1, -5, std::numeric_limits<int8_t>::min()});
}

TEST(fable_schema_number, validate_u64_narrow_high) {
  auto schema = fable::schema::Number<uint64_t>(nullptr, "")
                    .minimum(std::numeric_limits<uint64_t>::max() - 2);

  assert_validate_t<uint64_t>(
      schema, {std::numeric_limits<uint64_t>::max() - 2, std::numeric_limits<uint64_t>::max()});
  assert_invalidate_t<uint64_t>(schema, {0, std::numeric_limits<uint64_t>::max() - 3});
  assert_invalidate_t<uint32_t>(schema, {0, std::numeric_limits<uint32_t>::max()});
  assert_invalidate_t<int32_t>(schema, {-1, std::numeric_limits<int32_t>::max()});
}

TEST(fable_schema_number, validate_u8) {
  auto schema = fable::schema::Number<uint8_t>(nullptr, "");

  assert_validate_t<uint8_t>(schema, {0, 1, std::numeric_limits<uint8_t>::max()});
  assert_validate_t<int8_t>(schema, {0, 1, std::numeric_limits<int8_t>::max()});

  assert_invalidate_t<uint32_t>(schema, {std::numeric_limits<uint32_t>::max()});
  assert_invalidate_t<int8_t>(schema, {-1, -5, std::numeric_limits<int8_t>::min()});
  assert_invalidate_t<int64_t>(schema, {-1, -5, std::numeric_limits<int8_t>::min()});
}

TEST(fable_schema_number, validate_i64) {
  auto schema = fable::schema::Number<int64_t>(nullptr, "");

  assert_validate_t<uint64_t>(schema, {0, 1, std::numeric_limits<uint64_t>::max() / 2});
  assert_validate_t<int64_t>(schema, {0, 1, std::numeric_limits<int64_t>::max()});

  assert_invalidate_t<uint64_t>(schema, {std::numeric_limits<uint64_t>::max()});
}

TEST(fable_schema_number, validate_i8) {
  auto schema = fable::schema::Number<int8_t>(nullptr, "");

  assert_validate_t<uint8_t>(schema, {0, 1, std::numeric_limits<uint8_t>::max() / 2});
  assert_validate_t<int8_t>(schema, {-1, 0, 1, std::numeric_limits<int8_t>::max()});

  assert_invalidate_t<uint64_t>(schema, {std::numeric_limits<uint32_t>::max()});
  assert_invalidate_t<int64_t>(
      schema, {std::numeric_limits<int16_t>::min(), std::numeric_limits<int64_t>::max()});
}
