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
 * \file fable/confable.cpp
 * \see  fable/confable.cpp
 * \see  fable/schema.hpp
 */

#include <fable/confable.hpp>

#include <fable/conf.hpp>    // for Conf
#include <fable/schema.hpp>  // for Schema

namespace fable {

Confable::Confable() noexcept {};

Confable::Confable(const Confable&) noexcept : schema_(nullptr) {}

Confable::Confable(Confable&&) noexcept : schema_(nullptr) {}

Confable& Confable::operator=(const Confable& other) noexcept {
  if (this != &other) {
    schema_.reset();
  }
  return *this;
}

Confable& Confable::operator=(Confable&& other) noexcept {
  if (this != &other) {
    schema_.reset();
  }
  return *this;
}

Confable::~Confable() noexcept {};

void Confable::reset_schema() { schema_.reset(); }

Schema& Confable::schema() {
  if (!schema_) {
    schema_ = std::make_unique<Schema>(schema_impl());
  }
  return *schema_.get();
}

const Schema& Confable::schema() const { return const_cast<Confable*>(this)->schema(); }

void Confable::validate_or_throw(const Conf& c) const {
  std::optional<SchemaError> err;
  if (!validate(c, err)) {
    throw std::move(*err);
  }
}

bool Confable::validate(const Conf& c, std::optional<SchemaError>& err) const {
  return schema().validate(c, err);
}

void Confable::from_conf(const Conf& c) {
  validate_or_throw(c);
  schema().from_conf(c);
  reset_schema();
}

void Confable::to_json(Json& j) const { schema().to_json(j); }

Json Confable::to_json() const {
  Json j;
  to_json(j);
  return j;
}

Schema Confable::schema_impl() { return schema::Struct(); }

}  // namespace fable
