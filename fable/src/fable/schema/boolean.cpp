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
 * \file fable/schema/boolean.cpp
 * \see  fable/schema/boolean.hpp
 */

#include <fable/schema/boolean.hpp>

#include <string>   // for string
#include <utility>  // for move

namespace fable {
namespace schema {

Boolean::Boolean(Boolean::Type* ptr, std::string desc)
    : Base(JsonType::boolean, std::move(desc)), ptr_(ptr) {}

Json Boolean::json_schema() const {
  Json j{
      {"type", "boolean"},
  };
  this->augment_schema(j);
  return j;
}

bool Boolean::validate(const Conf& c, std::optional<SchemaError>& err) const {
  return this->validate_type(c, err);
}

void Boolean::to_json(Json& j) const {
  assert(ptr_ != nullptr);
  j = serialize(*ptr_);
}

void Boolean::from_conf(const Conf& c) {
  assert(ptr_ != nullptr);
  *ptr_ = deserialize(c);
}

Json Boolean::serialize(const Type& x) const { return x; }

void Boolean::serialize_into(Json& j, const Type& x) const { j = x; }

Boolean::Type Boolean::deserialize(const Conf& c) const { return c.get<Type>(); }

void Boolean::deserialize_into(const Conf& c, Type& x) const { x = c.get<Type>(); }

void Boolean::reset_ptr() { ptr_ = nullptr; }

}  // namespace schema
}  // namespace fable
