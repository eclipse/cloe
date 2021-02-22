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
 * \file fable/schema_test.cpp
 * \see  fable/schema.hpp
 */

#include <string>  // for string

#include <gtest/gtest.h>

#include <fable/confable.hpp>       // for Confable
#include <fable/schema.hpp>         // for Schema
#include <fable/utility/gtest.hpp>  // for schema_eq, ...

namespace {

enum class MyEnum { Enable, Disable };

// clang-format off
ENUM_SERIALIZATION(MyEnum, ({
    {MyEnum::Enable, "enable"},
    {MyEnum::Disable, "disable"},
}))
// clang-format on

struct ShouldCompile : public fable::Confable {
  std::vector<std::string> vehicles;

  CONFABLE_SCHEMA(ShouldCompile) {
    return fable::Schema{
        {"vehicles", fable::Schema(&vehicles, "")},
    };
  };
};

struct MyStruct : public fable::Confable {
  bool my_required = false;
  std::string my_string = "";
  int my_int = 0;
  std::string my_object_field = "";
  bool my_object_bool = false;
  MyEnum my_enum = MyEnum::Disable;
  boost::optional<std::string> middlename;

  bool not_applicable = true;
  std::string applicable = "";

  CONFABLE_SCHEMA(MyStruct) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"author", make_const_schema(std::string("me"), "author of this code")},
        {"required", make_schema(&my_required, "my required boolean, should be true").require()},
        {"string", make_schema(&my_string, "my string")},
        {"int", make_schema(&my_int, "my integer").minimum(0)},
        {"enum", make_schema(&my_enum, "my enum")},
        {
            "object",
            Struct{
                {"field", make_schema(&my_object_field, "my object string")},
                {"bool", make_schema(&my_object_bool, "my object boolean")},
            },
        },
        {
            "applicable",
            Variant{
                make_schema(&applicable, "string when applicable"),
                make_schema(&not_applicable, "false when not applicable"),
            },
        },
        {"middlename", make_schema(&middlename, "your middlename, maybe")},
        {"args", Ignore("args for someone else")},
    };
  };

  void to_json(fable::Json& j) const override {
    j = fable::Json{
        {"author", "me"},
        {"required", my_required},
        {"string", my_string},
        {"int", my_int},
        {"enum", my_enum},
        {"object",
         {
             {"field", my_object_field},
             {"bool", my_object_bool},
         }},
        {"applicable", applicable},
    };
    if (middlename) {
      j["middlename"] = middlename.get();
    }
  }
};

}  // anonymous namespace

TEST(fable_schema, schema_wrapper) {
  using namespace fable;          // NOLINT(build/namespaces)
  using namespace fable::schema;  // NOLINT(build/namespaces)

  bool my_required = false;
  std::string my_string = "";
  int my_int = 0;
  std::string my_object_field = "";
  bool my_object_bool = false;
  MyEnum my_enum = MyEnum::Disable;
  bool not_applicable = true;
  std::string applicable = "";
  boost::optional<std::string> middlename;

  auto s1 = Schema{
      {"author", make_const_str("me", "author of this code")},
      {"required", make_schema(&my_required, "my required boolean, should be true").require()},
      {"string", Schema(&my_string, "my string")},
      {"int", make_schema(&my_int, "my integer").minimum(0)},
      {"enum", Schema(&my_enum, "my enum")},
      {
          "object",
          Schema{
              {"field", Schema(&my_object_field, "my object string")},
              {"bool", Schema(&my_object_bool, "my object boolean")},
          },
      },
      {
          "applicable",
          Schema{
              Schema(&applicable, "string when applicable"),
              Schema(&not_applicable, "false when not applicable"),
          },
      },
      {"middlename", Schema(&middlename, "your middlename, maybe")},
      {"args", Schema("args for someone else")},
  };

  MyStruct s2;

  fable::assert_eq(s1.json_schema(), s2.schema().json_schema());
}

TEST(fable_schema, json_schema) {
  MyStruct tmp;
  ASSERT_FALSE(tmp.my_required);
  ASSERT_EQ(tmp.my_string, "");
  ASSERT_EQ(tmp.my_int, 0);
  ASSERT_EQ(tmp.my_enum, MyEnum::Disable);
  ASSERT_EQ(tmp.my_object_field, "");
  ASSERT_FALSE(tmp.my_object_bool);

  fable::assert_schema_eq(tmp, R"({
    "additionalProperties": false,
    "properties": {
      "args": {
        "description": "args for someone else"
      },
      "author": {
        "description": "author of this code",
        "const": "me"
      },
      "int": {
        "description": "my integer",
        "maximum": 2147483647,
        "minimum": 0,
        "type": "integer"
      },
      "enum": {
        "description": "my enum",
        "type": "string",
        "enum": ["enable", "disable"]
      },
      "object": {
        "additionalProperties": false,
        "properties": {
          "bool": {
            "description": "my object boolean",
            "type": "boolean"
          },
          "field": {
            "description": "my object string",
            "type": "string"
          }
        },
        "type": "object"
      },
      "required": {
        "description": "my required boolean, should be true",
        "type": "boolean"
      },
      "string": {
        "description": "my string",
        "type": "string"
      },
      "applicable": {
        "anyOf": [
          { "type": "string", "description": "string when applicable" },
          { "type": "boolean", "description": "false when not applicable" }
        ]
      },
      "middlename": {
        "description": "your middlename, maybe",
        "oneOf": [
          { "type": "null" },
          { "type": "string" }
        ]
      }
    },
    "required": [
      "required"
    ],
    "type": "object"
  })");

  fable::assert_eq(tmp.schema().to_json(), tmp.to_json());
}

TEST(fable_schema, require_false) {
  MyStruct tmp;
  fable::assert_from_conf_throw(tmp, R"(
    {
      "int": 5
    }
  )");
  ASSERT_EQ(tmp.my_int, 0);
}

TEST(fable_schema, require_true) {
  MyStruct tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": true
    }
  )");
  ASSERT_TRUE(tmp.my_required);
}

TEST(fable_schema, tolerate_false) {
  MyStruct tmp;
  fable::assert_from_conf_throw(tmp, R"(
    {
      "required": false,
      "unknown": false
    }
  )");
}

TEST(fable_schema, tolerate_true) {
  MyStruct tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": false,
      "args": {
        "ignore": "me",
        "and my": "friends",
        "ok": true
      }
    }
  )");
}

TEST(fable_schema, set_primitive_bool) {
  MyStruct tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": true
    }
  )");
  ASSERT_TRUE(tmp.my_required);
}

TEST(fable_schema, set_primitive_int) {
  MyStruct tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": true,
      "int": 42
    }
  )");
  ASSERT_TRUE(tmp.my_required);
  ASSERT_EQ(tmp.my_int, 42);
}

TEST(fable_schema, set_primitive_string) {
  MyStruct tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": true,
      "string": "string"
    }
  )");
  ASSERT_TRUE(tmp.my_required);
  ASSERT_EQ(tmp.my_string, "string");
}

TEST(fable_schema, set_primitive_enum) {
  MyStruct tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": true,
      "enum": "enable"
    }
  )");
  ASSERT_TRUE(tmp.my_required);
  ASSERT_EQ(tmp.my_enum, MyEnum::Enable);
}

TEST(fable_schema, set_object) {
  MyStruct tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": true,
      "object": {
        "field": "field",
        "bool": true
      }
    }
  )");
  ASSERT_TRUE(tmp.my_required);
  ASSERT_EQ(tmp.my_object_field, "field");
  ASSERT_TRUE(tmp.my_object_bool);
}

TEST(fable_schema, set_copy_object) {
  MyStruct original;
  MyStruct copy = original;

  fable::assert_from_conf(copy, R"(
    {
      "required": true,
      "object": {
        "field": "field",
        "bool": true
      }
    }
  )");

  // When modifying copy, only copy should be modified, not original.
  fable::assert_ne(original.to_json(), copy.to_json());
  ASSERT_TRUE(copy.my_required);
  ASSERT_EQ(copy.my_object_field, "field");
  ASSERT_TRUE(copy.my_object_bool);
  ASSERT_FALSE(original.my_required);
}

namespace {

struct MyDerived : public MyStruct {
  bool my_derived = false;

  CONFABLE_SCHEMA(MyDerived) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        MyStruct::schema_impl(),
        {
            {"derived", make_schema(&my_derived, "my derived boolean, should be true").require()},
        },
    };
  }
};

}  // namespace

TEST(fable_schema, set_derived_object) {
  MyDerived tmp;
  fable::assert_from_conf(tmp, R"(
    {
      "required": true,
      "derived": true,
      "object": {
        "field": "field",
        "bool": true
      }
    }
  )");

  ASSERT_TRUE(tmp.my_required);
  ASSERT_TRUE(tmp.my_derived);
  ASSERT_EQ(tmp.my_object_field, "field");
  ASSERT_TRUE(tmp.my_object_bool);
}
