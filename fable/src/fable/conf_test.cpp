/*
 * Copyright 2024 Robert Bosch GmbH
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
 * \file fable/environment_test.cpp
 * \see  fable/environment.hpp
 */

#include <gtest/gtest.h>

#include <fable/conf.hpp>
#include <fable/json.hpp>
#include <fable/utility/gtest.hpp>

using namespace fable;

class fable_conf : public testing::Test {
 protected:
  Conf conf;

  void SetUp() override {
    conf = Conf{Json{
        {"foo",
         {
             {"bar", 42},
             {"baz", {}}, // null
         }},
    }};
  }
};

TEST_F(fable_conf, erase_nonexistant) {
  ASSERT_EQ(conf.erase("noexists"), 0);
  ASSERT_EQ(conf.erase_pointer("/noexists"), 0);
  ASSERT_EQ(conf.erase_pointer("/nested/noexists"), 0);
  ASSERT_EQ(conf.erase_pointer("/foo/noexists"), 0);
}

TEST_F(fable_conf, erase_simple) {
  ASSERT_EQ(conf.erase("foo"), 1);
  assert_eq(*conf, Json::object());
}

TEST_F(fable_conf, erase_preserve_empty) {
  ASSERT_EQ(conf.erase_pointer("/foo/baz"), 1);
  ASSERT_EQ(conf.erase_pointer("/foo/bar", true), 1);
  assert_eq(*conf, {{"foo",Json::object()}});
  ASSERT_EQ(conf.erase_pointer("/foo"), 1);
  assert_eq(*conf, Json::object());
}

TEST_F(fable_conf, erase_all) {
  ASSERT_EQ(conf.erase_pointer("/foo/baz"), 1);
  ASSERT_EQ(conf.erase_pointer("/foo/bar"), 2);
  assert_eq(*conf, Json::object());
}

TEST_F(fable_conf, erase_invalid) {
  ASSERT_EQ(conf.erase(""), 0);
  ASSERT_EQ(conf.erase_pointer("/"), 0);
}

TEST_F(fable_conf, has) {
  ASSERT_TRUE(conf.has("foo"));
  ASSERT_FALSE(conf.has("non-existant"));
  ASSERT_TRUE(conf.has_pointer("/foo/bar"));
  ASSERT_TRUE(conf.has_pointer("/foo/baz"));
  ASSERT_FALSE(conf.has_pointer("/foo/non-existant"));
}

TEST_F(fable_conf, try_from) {
  int x = 17;
  ASSERT_NO_THROW(conf.try_from_pointer("/foo/noexists", x));
  ASSERT_EQ(x, 17);
  ASSERT_NO_THROW(conf.try_from_pointer("/foo/bar", x));
  ASSERT_EQ(x, 42);
}

TEST_F(fable_conf, at_null) {
  auto sub = conf.at_pointer("/foo/baz");
  assert_eq(*sub, Json{});
  auto j = conf->at(JsonPointer("/foo/baz"));
  ASSERT_EQ(j.type(), JsonType::null);
  ASSERT_THROW(j.get<int>(), Json::type_error);
}

TEST_F(fable_conf, get_optional) {
  ASSERT_EQ(conf.get_pointer<std::optional<int>>("/foo/baz"), std::optional<int>());
}

TEST_F(fable_conf, get_null_as_int) {
  auto tmp1 = conf.at_pointer("/foo/baz");
  ASSERT_THROW({std::ignore = tmp1.get<int>(); }, ConfError);

  auto tmp2 = (*conf).at(JsonPointer("/foo/baz"));
  ASSERT_THROW({std::ignore = tmp2.get<int>(); }, Json::type_error);

  ASSERT_THROW({ std::ignore = conf.get_pointer<int>("/foo/baz"); }, ConfError);
}

TEST_F(fable_conf, try_from_null) {
  int x = 17;
  ASSERT_THROW(conf.try_from_pointer("/foo/baz", x), ConfError);
}
