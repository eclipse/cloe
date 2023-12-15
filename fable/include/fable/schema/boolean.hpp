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
 * \file fable/schema/boolean.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 */

#pragma once

#include <string>   // for string
#include <utility>  // for move

#include <fable/schema/interface.hpp>  // for Base<>

namespace fable {
namespace schema {

class Boolean : public Base<Boolean> {
 public:  // Types and Constructors
  using Type = bool;

  Boolean(Type* ptr, std::string&& desc);

 public:  // Overrides
  Json json_schema() const override;
  void validate(const Conf& c) const override;
  using Interface::to_json;
  void to_json(Json& j) const override;
  void from_conf(const Conf& c) override;
  Json serialize(const Type& x) const;
  Type deserialize(const Conf& c) const;
  void reset_ptr() override;

 private:
  Type* ptr_{nullptr};
};

inline Boolean make_schema_impl(bool* ptr, std::string&& desc) { return Boolean(ptr, std::move(desc)); }

}  // namespace schema
}  // namespace fable
