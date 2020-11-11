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
 * \file fable/schema.cpp
 * \see  fable/schema.hpp
 */

#include <fable/schema.hpp>

#include <string>  // for string

namespace fable {

using schema::BoxList;
using schema::BoxVec;
using schema::Variant;

namespace {

BoxVec to_box_vec(const std::vector<Schema>& xs) {
  BoxVec out;
  out.reserve(xs.size());
  for (const auto& x : xs) {
    out.emplace_back(x);
  }
  return out;
}

}  // anonymous namespace

// Variant constructions:
Schema::Schema(const std::vector<Schema>& xs) : impl_(new Variant(to_box_vec(xs))) {}
Schema::Schema(std::string&& desc, const std::vector<Schema>& xs)
    : impl_(new Variant(std::move(desc), to_box_vec(xs))) {}
Schema::Schema(schema::BoxList props) : impl_(new Variant(props)) {}
Schema::Schema(std::string&& desc, schema::BoxList props)
    : impl_(new Variant(std::move(desc), props)) {}
Schema::Schema(schema::BoxVec&& props) : impl_(new Variant(std::move(props))) {}
Schema::Schema(std::string&& desc, schema::BoxVec&& props)
    : impl_(new Variant(std::move(desc), std::move(props))) {}

}  // namespace fable
