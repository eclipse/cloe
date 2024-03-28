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
/**
 * \file fable/schema/path_test.cpp
 * \see  fable/schema/string.hpp
 */

#include <filesystem>

#include <gtest/gtest.h>

#include <fable/confable.hpp>       // for Confable
#include <fable/schema/path.hpp>  // for Path, ...
#include <fable/utility/gtest.hpp>  // for assert_to_json, ...

#define TO_CONF(x) fable::Conf{fable::Json(x)}

TEST(fable_schema_path, executable) {
  std::filesystem::path p;
  auto s = fable::schema::make_schema(&p, "executable").executable();

  // Should be something like /usr/bin/echo, but because this is
  // system dependent we just exect it to succeed.
  fable::assert_from_conf(s, TO_CONF("echo"));
  fable::assert_schema_eq(s, R"({
    "type": "string",
    "comment": "path should be executable",
    "description": "executable"
  })");
}
