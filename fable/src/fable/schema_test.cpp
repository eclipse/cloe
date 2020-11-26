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

#include <fable/confable.hpp>  // for Confable
#include <fable/schema.hpp>    // for Schema
using namespace fable;         // NOLINT(build/namespaces)

namespace {

enum class MyEnum { Enable, Disable };

// clang-format off
ENUM_SERIALIZATION(MyEnum, ({
    {MyEnum::Enable, "enable"},
    {MyEnum::Disable, "disable"},
}))
// clang-format on

struct ShouldCompile : public Confable {
  std::vector<std::string> vehicles;

  CONFABLE_SCHEMA(ShouldCompile) {
    return Schema{
        {"vehicles", Schema(&vehicles, "")},
    };
  };
};

struct MyStruct : public Confable {
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

  void to_json(Json& j) const override {
    j = Json{
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

  ASSERT_EQ(s1.json_schema().dump(), s2.schema().json_schema().dump());
}

TEST(fable_schema, json_schema) {
  MyStruct s;
  ASSERT_FALSE(s.my_required);
  ASSERT_EQ(s.my_string, "");
  ASSERT_EQ(s.my_int, 0);
  ASSERT_EQ(s.my_enum, MyEnum::Disable);
  ASSERT_EQ(s.my_object_field, "");
  ASSERT_FALSE(s.my_object_bool);

  Json schema = R"({
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
  })"_json;

  auto j1 = s.schema().to_json().dump();
  auto j2 = s.to_json().dump();
  ASSERT_EQ(j1, j2);

  auto s1 = schema.dump();
  auto s2 = s.schema().json_schema().dump();
  ASSERT_EQ(s1, s2);
}

TEST(fable_schema, require_false) {
  MyStruct s;
  Conf input{R"(
    {
      "int": 5
    }
  )"_json};

  ASSERT_ANY_THROW(s.from_conf(input));
  ASSERT_EQ(s.my_int, 0);
}

TEST(fable_schema, require_true) {
  MyStruct s;
  Conf input{R"(
    {
      "required": true
    }
  )"_json};

  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
}

TEST(fable_schema, tolerate_false) {
  MyStruct s;
  Conf input{R"(
    {
      "required": false,
      "unknown": false
    }
  )"_json};

  ASSERT_ANY_THROW(s.from_conf(input));
}

TEST(fable_schema, tolerate_true) {
  MyStruct s;
  Conf input{R"(
    {
      "required": false,
      "args": {
        "ignore": "me",
        "and my": "friends",
        "ok": true
      }
    }
  )"_json};

  s.from_conf(input);
}

TEST(fable_schema, set_primitive_bool) {
  MyStruct s;
  Conf input{R"(
    {
      "required": true
    }
  )"_json};

  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
}

TEST(fable_schema, set_primitive_int) {
  MyStruct s;
  Conf input{R"(
    {
      "required": true,
      "int": 42
    }
  )"_json};

  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
  ASSERT_EQ(s.my_int, 42);
}

TEST(fable_schema, set_primitive_string) {
  MyStruct s;
  Conf input{R"(
    {
      "required": true,
      "string": "string"
    }
  )"_json};

  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
  ASSERT_EQ(s.my_string, "string");
}

TEST(fable_schema, set_primitive_enum) {
  MyStruct s;
  Conf input{R"(
    {
      "required": true,
      "enum": "enable"
    }
  )"_json};

  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
  ASSERT_EQ(s.my_enum, MyEnum::Enable);
}

TEST(fable_schema, set_object) {
  MyStruct s;
  Conf input{R"(
    {
      "required": true,
      "object": {
        "field": "field",
        "bool": true
      }
    }
  )"_json};

  s.schema().validate(input);
  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
  ASSERT_EQ(s.my_object_field, "field");
  ASSERT_TRUE(s.my_object_bool);
}

TEST(fable_schema, set_copy_object) {
  MyStruct t;
  MyStruct s = t;
  Conf input{R"(
    {
      "required": true,
      "object": {
        "field": "field",
        "bool": true
      }
    }
  )"_json};

  // When modifying s, only s should be modified, not t.
  s.schema().validate(input);
  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
  ASSERT_EQ(s.my_object_field, "field");
  ASSERT_TRUE(s.my_object_bool);
  ASSERT_FALSE(t.my_required);
}

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

TEST(fable_schema, set_derived_object) {
  MyDerived s;
  Conf input{R"(
    {
      "required": true,
      "derived": true,
      "object": {
        "field": "field",
        "bool": true
      }
    }
  )"_json};

  s.from_conf(input);
  ASSERT_TRUE(s.my_required);
  ASSERT_TRUE(s.my_derived);
  ASSERT_EQ(s.my_object_field, "field");
  ASSERT_TRUE(s.my_object_bool);
}
