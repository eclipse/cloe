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

namespace fable {
namespace schema {

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
  CustomDeserializer(Box&& s, std::function<void(CustomDeserializer*, const Conf&)> f)
      : impl_(std::move(s).reset_pointer().get()), from_conf_fn_(f) {}

 public:  // Special
  operator Box() const { return Box{this->clone()}; }

  void set_from_conf(std::function<void(CustomDeserializer*, const Conf&)> f) { from_conf_fn_ = f; }

  CustomDeserializer with_from_conf(std::function<void(CustomDeserializer*, const Conf&)> f) && {
    set_from_conf(f);
    return std::move(*this);
  }

 public:  // Overrides
  Interface* clone() const override { return new CustomDeserializer(*this); }

  using Interface::to_json;
  JsonType type() const override { return impl_->type(); }
  std::string type_string() const override { return impl_->type_string(); }
  bool is_required() const override { return impl_->is_required(); }
  const std::string& description() const override { return impl_->description(); }
  void set_description(std::string s) override { return impl_->set_description(std::move(s)); }
  Json usage() const override { return impl_->usage(); }
  Json json_schema() const override { return impl_->json_schema(); };
  void validate(const Conf& c) const override { impl_->validate(c); }
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

  // TODO: Implement or explain why we don't need the following methods:
  // - serialize
  // - serialize_into
  // - deserialize
  // - deserialize_into

 private:
  std::shared_ptr<schema::Interface> impl_{nullptr};
  std::function<void(CustomDeserializer*, const Conf&)> from_conf_fn_{};
};

}  // namespace schema
}  // namespace fable
