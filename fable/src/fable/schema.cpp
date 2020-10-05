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

using schema::BoxMap;
using schema::BoxPairList;
using schema::Struct;

using schema::BoxList;
using schema::BoxVec;
using schema::Variant;

using SchemaMap = Schema::SchemaMap;
using SchemaVec = Schema::SchemaVec;

namespace {

BoxMap to_box_map(const SchemaMap& m) {
  BoxMap out;
  for (const auto& kv : m) {
    out.insert(std::make_pair(kv.first, kv.second));
  }
  return out;
}

BoxVec to_box_vec(const SchemaVec& xs) {
  BoxVec out;
  out.reserve(xs.size());
  for (const auto& x : xs) {
    out.emplace_back(x);
  }
  return out;
}

}  // anonymous namespace

// Struct constructions:
Schema::Schema(const SchemaMap& props) : Schema("", std::move(props)) {}
Schema::Schema(std::string&& desc, const SchemaMap& props)
    : impl_(new Struct(std::move(desc), to_box_map(props))) {}
Schema::Schema(BoxPairList props) : Schema("", std::move(props)) {}
Schema::Schema(std::string&& desc, BoxPairList props)
    : impl_(new Struct(std::move(desc), std::move(props))) {}
Schema::Schema(const Schema& base, BoxPairList props) : Schema("", base, std::move(props)) {}
Schema::Schema(std::string&& desc, const Schema& base, BoxPairList props)
    : impl_(new Struct(std::move(desc), base, std::move(props))) {}

// Variant constructions:
Schema::Schema(const SchemaVec& xs) : impl_(new Variant(to_box_vec(xs))) {}
Schema::Schema(std::string&& desc, const SchemaVec& xs)
    : impl_(new Variant(std::move(desc), to_box_vec(xs))) {}
Schema::Schema(schema::BoxList props) : impl_(new Variant(props)) {}
Schema::Schema(std::string&& desc, schema::BoxList props)
    : impl_(new Variant(std::move(desc), props)) {}
Schema::Schema(schema::BoxVec&& props) : impl_(new Variant(std::move(props))) {}
Schema::Schema(std::string&& desc, schema::BoxVec&& props)
    : impl_(new Variant(std::move(desc), std::move(props))) {}

}  // namespace fable
