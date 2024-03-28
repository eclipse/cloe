/*
 * Copyright 2021 Robert Bosch GmbH
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
 * \file fable/schema/custom.hpp
 * \see  fable/schema/custom_test.cpp
 * \see  fable/schema/variant.hpp
 */

#pragma once

#include <functional>  // for function<>

#include <fable/schema/interface.hpp>  // for Interface, Box

namespace fable::schema {

/**
 * The CustomDeserializer allows the user to replace the deserialization of a
 * Schema with a custom function.
 *
 * This is especially useful for schema types such as Variants.
 *
 * Note that if you use this, you may have to define to_json yourself.
 */
class CustomDeserializer : public schema::Interface {
 public:  // Constructors
  CustomDeserializer(Box&& s) : impl_(std::move(s).reset_pointer().get()) {}
  CustomDeserializer(Box&& s, std::function<void(CustomDeserializer*, const Conf&)> deserialize_fn)
      : impl_(std::move(s).reset_pointer().get()), from_conf_fn_(std::move(deserialize_fn)) {}

 public:  // Special
  [[nodiscard]] operator Box() const { return Box{this->clone()}; }

  void set_from_conf(std::function<void(CustomDeserializer*, const Conf&)> deserialize_fn) {
    from_conf_fn_ = std::move(deserialize_fn);
  }

  CustomDeserializer with_from_conf(
      std::function<void(CustomDeserializer*, const Conf&)> deserialize_fn) && {
    set_from_conf(std::move(deserialize_fn));
    return std::move(*this);
  }

 public:  // Overrides
  [[nodiscard]] std::unique_ptr<Interface> clone() const override {
    return std::make_unique<CustomDeserializer>(*this);
  }

  using Interface::to_json;
  [[nodiscard]] JsonType type() const override { return impl_->type(); }
  [[nodiscard]] std::string type_string() const override { return impl_->type_string(); }
  [[nodiscard]] bool is_required() const override { return impl_->is_required(); }
  [[nodiscard]] const std::string& description() const override { return impl_->description(); }
  void set_description(std::string s) override { return impl_->set_description(std::move(s)); }
  [[nodiscard]] Json usage() const override { return impl_->usage(); }
  [[nodiscard]] Json json_schema() const override { return impl_->json_schema(); };
  bool validate(const Conf& c, std::optional<SchemaError>& err) const override {
    return impl_->validate(c, err);
  }
  void to_json(Json& j) const override { impl_->to_json(j); }

  void from_conf(const Conf& c) override {
    if (!from_conf_fn_) {
      throw Error("no deserializer configured");
    }
    from_conf_fn_(this, c);
  }

  void reset_ptr() override {
    impl_->reset_ptr();
    from_conf_fn_ = [](CustomDeserializer*, const Conf&) {
      throw Error("cannot deserialize after reset_ptr is called");
    };
  }

  friend void to_json(Json& j, const CustomDeserializer& b) { b.impl_->to_json(j); }

  // NOTE: The following methods cannot be implemented because
  // CustomDeserializer does not have an associated type:
  //
  //     Json serialize(const Type&) const;
  //     void serialize_into(Json&, const Type&) const;
  //     Type deserialize(const Conf&) const;
  //     void deserialize_into(const Conf&, Type&) const;
  //
  // This means that `CustomDeserializer` cannot be used as a prototype schema
  // directly, for example with `optional`. Use `Confable` instead.

 private:
  std::shared_ptr<schema::Interface> impl_{nullptr};
  std::function<void(CustomDeserializer*, const Conf&)> from_conf_fn_{};
};

}  // namespace fable::schema
