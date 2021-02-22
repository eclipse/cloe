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
 * \file fable/schema/struct_test.cpp
 * \see  fable/schema/struct.hpp
 */

#include <string>  // for string

#include <gtest/gtest.h>  // for TEST, ...

#include <fable/confable.hpp>       // for Confable
#include <fable/schema.hpp>         // for Schema, Struct, ...
#include <fable/utility/gtest.hpp>  // for assert_schema_eq, ...

namespace {

struct StructA : public fable::Confable {
  int a = 0;

  CONFABLE_SCHEMA(StructA) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"a", make_schema(&a, "integer")},
    };
  }
};

struct StructB : public fable::Confable {
  int b = 0;
  CONFABLE_SCHEMA(StructB) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"b", make_schema(&b, "integer").require()},
    };
  }
};

struct StructFoo : public fable::Confable {
  StructA a;
  StructB b;

  CONFABLE_SCHEMA(StructFoo) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct().properties_from(&a).properties_from(&b);
  }
};

}  // anonymous namespace

TEST(fable_schema_struct, foo_schema) {
  StructFoo tmp;
  fable::assert_schema_eq(tmp, R"({
    "additionalProperties": false,
    "properties": {
      "a": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      },
      "b": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      }
    },
    "required": [
      "b"
    ],
    "type": "object"
  })");
}

TEST(fable_schema_struct, foo_validate) {
  StructFoo tmp;

  fable::assert_validate(tmp, R"({
    "a": 1,
    "b": 3
  })");
  ASSERT_EQ(tmp.a.a, 0);

  fable::assert_validate(tmp, R"({
    "b": 5
  })");
  ASSERT_EQ(tmp.b.b, 0);

  fable::assert_invalidate(tmp, R"({
    "a": 3
  })");

  fable::assert_invalidate(tmp, R"({
    "b": "string"
  })");
}

TEST(fable_schema_struct, foo_to_json) {
  StructFoo tmp;
  tmp.a.a = 1;
  tmp.b.b = 42;
  fable::assert_to_json(tmp, R"({
    "a": 1,
    "b": 42
  })");
}

TEST(fable_schema_struct, foo_from_conf) {
  StructFoo tmp;
  fable::assert_from_conf(tmp, R"({
    "b": 42
  })");
  fable::assert_to_json(tmp, R"({
    "a": 0,
    "b": 42
  })");
}

// ------------------------------------------------------------------------- //

namespace {

struct StructWithAdditionalPrototype : public fable::Confable {
  int32_t a;
  bool b;

  CONFABLE_SCHEMA(StructWithAdditionalPrototype) {
    using namespace fable::schema;  // NOLINT(build/namespace)
    return Struct{
        {"a", make_schema(&a, "integer")},
        {"b", make_schema(&b, "bool")},
    }
        .additional_properties(make_prototype<std::string>());
  }
};

}  // namespace

TEST(fable_schema_struct, additional_prototype_schema) {
  StructWithAdditionalPrototype tmp;
  fable::assert_schema_eq(tmp, R"({
    "additionalProperties": {
      "type": "string"
    },
    "properties": {
      "a": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      },
      "b": {
        "description": "bool",
        "type": "boolean"
      }
    },
    "type": "object"
  })");
}

// ------------------------------------------------------------------------- //

namespace {

struct StructBase : public fable::Confable {
  virtual ~StructBase() = default;

  int base = 0;

  CONFABLE_SCHEMA(StructBase) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        {"base", make_schema(&base, "integer")},
    };
  }
};

}  // anonymous namespace

TEST(fable_schema_struct, base_schema) {
  StructBase base;
  fable::assert_schema_eq(base, R"({
    "additionalProperties": false,
    "properties": {
      "base": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      }
    },
    "type": "object"
  })");
}

TEST(fable_schema_struct, base_from_eq_to) {
  StructBase tmp;
  fable::assert_from_eq_to(tmp, R"({
    "base": 42
  })");
  ASSERT_EQ(tmp.base, 42);
}

namespace {

struct StructSub1 : public StructBase {
  virtual ~StructSub1() = default;

  bool sub = 1;

  CONFABLE_SCHEMA(StructSub1) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        StructBase::schema_impl(),
        {
            {"sub", make_schema(&sub, "boolean").require()},
        },
    };
  }
};

}  // anonymous namespace

TEST(fable_schema_struct, base_sub1_schema) {
  StructSub1 sub1;
  fable::assert_schema_eq(sub1, R"({
    "additionalProperties": false,
    "properties": {
      "base": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      },
      "sub": {
        "description": "boolean",
        "type": "boolean"
      }
    },
    "required": ["sub"],
    "type": "object"
  })");
}

TEST(fable_schema_struct, base_sub1_from_eq_to) {
  StructSub1 tmp;
  fable::assert_from_eq_to(tmp, R"({
    "base": 42,
    "sub": true
  })");
  ASSERT_EQ(tmp.base, 42);
  ASSERT_EQ(tmp.sub, true);
}

namespace {

struct StructSub2 : public StructBase {
  std::string sub;

  CONFABLE_SCHEMA(StructSub2) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        StructBase::schema_impl(),
        {
            {"sub", make_schema(&sub, "string")},
        },
    };
  }
};

}  // anonymous namespace

TEST(fable_schema_struct, base_sub2_schema) {
  StructSub2 sub2;
  fable::assert_schema_eq(sub2, R"({
    "additionalProperties": false,
    "properties": {
      "base": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      },
      "sub": {
        "description": "string",
        "type": "string"
      }
    },
    "type": "object"
  })");
}

TEST(fable_schema_struct, base_sub2_from_eq_to) {
  StructSub2 tmp;
  fable::assert_from_eq_to(tmp, R"({
    "base": 42,
    "sub": "hello world"
  })");
  ASSERT_EQ(tmp.base, 42);
  ASSERT_EQ(tmp.sub, "hello world");
}

namespace {

struct StructSub1Sub1 : public StructSub1 {
  int foo = 2;

  CONFABLE_SCHEMA(StructSub1Sub1) {
    using namespace fable::schema;  // NOLINT(build/namespaces)
    return Struct{
        StructSub1::schema_impl(),
        {
            {"foo", make_schema(&foo, "integer")},
        },
    };
  }
};

}  // anonymous namespace

TEST(fable_schema_struct, base_sub1sub1_schema) {
  StructSub1Sub1 tmp;
  fable::assert_schema_eq(tmp, R"({
    "additionalProperties": false,
    "properties": {
      "base": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      },
      "foo": {
        "description": "integer",
        "maximum": 2147483647,
        "minimum": -2147483648,
        "type": "integer"
      },
      "sub": {
        "description": "boolean",
        "type": "boolean"
      }
    },
    "required": ["sub"],
    "type": "object"
  })");
}

TEST(fable_schema_struct, base_sub1sub1_from_eq_to) {
  StructSub1Sub1 tmp;
  fable::assert_from_eq_to(tmp, R"({
    "base": 42,
    "foo": 8,
    "sub": true
  })");
  ASSERT_EQ(tmp.base, 42);
  ASSERT_EQ(tmp.foo, 8);
  ASSERT_EQ(tmp.sub, true);
}
