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
 * \file fable/schema/factory.hpp
 * \see  fable/schema/factory_test.cpp
 */

#pragma once
#ifndef FABLE_SCHEMA_FACTORY_HPP_
#define FABLE_SCHEMA_FACTORY_HPP_

#include <functional>   // for function<>
#include <limits>       // for numeric_limits<>
#include <map>          // for map<>
#include <memory>       // for shared_ptr<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_base_of<>
#include <utility>      // for move
#include <vector>       // for vector<>

#include <fable/schema/const.hpp>      // for Const
#include <fable/schema/interface.hpp>  // for Base<>, Box
#include <fable/schema/string.hpp>     // for String
#include <fable/schema/struct.hpp>     // for Struct
#include <fable/schema/variant.hpp>    // for Variant

namespace fable {
namespace schema {

/**
 * FactoryBase is the base class for Factory and FactoryPointerless.
 *
 * It is a schema of schemas that can create different objects based on the
 * input. This is why there are two versions, since it may be used in
 * situations where it's not desired to serialize directly into a type via
 * pointer.
 *
 * Note that this class should not be used directly, instead use Factory or
 * FactoryPointerless. However, the interface of FactoryPointerless and Factory
 * are almost identical, with Factory provided three more constructors with
 * pointer arguments.
 */
template <typename T, typename CRTP>
class FactoryBase : public Base<CRTP> {
 public:  // Types
  using Type = T;
  using MakeFunc = std::function<T(const Conf& c)>;
  struct TypeFactory {
    TypeFactory(const Box& s, MakeFunc f) : schema(s), func(f) { schema.reset_ptr(); }

    Box schema;
    MakeFunc func;
  };

  using FactoryMap = std::map<std::string, TypeFactory>;
  using FactoryPairList = std::initializer_list<std::pair<std::string, TypeFactory>>;

 public:  // Constructors
  /**
   * Construct an empty factory.
   *
   * This schema is useless until you add some factories with the add_factory
   * method.
   *
   * \see add_factory()
   */
  explicit FactoryBase(std::string&& desc = "") : Base<CRTP>(JsonType::object, std::move(desc)) {}

  FactoryBase(std::string&& desc, FactoryPairList fs)
      : Base<CRTP>(JsonType::object, std::move(desc)), available_(std::move(fs)) {
    reset_schema();
  }

  FactoryBase(std::string&& desc, FactoryMap&& fs)
      : Base<CRTP>(JsonType::object, std::move(desc)), available_(std::move(fs)) {
    reset_schema();
  }

  /**
   * Set the factory key and return this for chaining.
   *
   * \see set_factory_key()
   */
  CRTP factory_key(const std::string& keyword) && {
    set_factory_key(keyword);
    return std::move(*dynamic_cast<CRTP*>(this));
  }

  /**
   * Set the args key and return this for chaining.
   *
   * \see set_args_key()
   */
  CRTP args_key(const std::string& keyword) && {
    set_args_key(keyword);
    return std::move(*dynamic_cast<CRTP*>(this));
  }

  /**
   * Set whether to return only the args subset and return this for chaining.
   *
   * \see set_args_subset()
   */
  CRTP args_subset(bool value) && {
    set_args_subset(value);
    return std::move(*dynamic_cast<CRTP*>(this));
  }

 public:  // Special
  /**
   * Set the factory key in the input that is used for selecting the
   * correct factory.
   *
   * Common choices could be: factory, type, binding.
   */
  void set_factory_key(const std::string& keyword) {
    assert(keyword != "");
    factory_key_ = keyword;
    reset_schema();
  }

  /**
   * Set the args key that is for selecting the input to pass on to the factory
   * function.
   *
   * - This affects the final schema used for validation.
   * - If the keyword is an empty string, the same object space is used as
   *   contains the factory keyword, so the factory schema should not use
   *   that keyword.
   */
  void set_args_key(const std::string& keyword) {
    args_key_ = keyword;
    reset_schema();
  }

  /**
   * Set whether only the args subset of the conf should be passed to the
   * factory function.
   *
   * The default behavior is true.
   *
   * - If true, only the args subset of incoming confs will be passed on to
   *   the factory function. If the args key is empty, then only the factory
   *   key is erased from the conf.
   * - If false, the incoming args is passed to the factory function as-is.
   */
  void set_args_subset(bool value) { args_subset_ = value; }

  /**
   * Return the schema and factory function associated with the given key.
   */
  const TypeFactory& get_factory(const std::string& key) const { return available_.at(key); }

  /**
   * Return whether a factory with the given key is available.
   */
  bool has_factory(const std::string& key) const { return available_.count(key); }

  /**
   * Add a factory with the given key, schema, and function.
   */
  void add_factory(const std::string& key, const Box& s, MakeFunc f) {
    available_.insert(std::make_pair(key, TypeFactory{s, f}));
    reset_schema();
  }

  /**
   * Add a factory which creates a default instance of F and calls from_conf
   * on F.
   *
   * - `F` must be default-constructible.
   * - `F` must have a `from_conf()` method.
   * - `F` must have a schema, i.e., `make_prototype<F>()` should be valid.
   * - `std::unique_ptr<F>` must be convertible to `T`.
   *   This means that T must be either a `std::unique_ptr<Base>` or
   *   `std::shared_ptr<Base>`, where `Base` is a base class of F.
   *
   * Most types inheriting from `Confable` will fulfill these requirements.
   */
  template <typename F,
            std::enable_if_t<(std::is_default_constructible<F>::value &&
                              std::is_convertible<std::unique_ptr<F>, T>::value),
                             int> = 0>
  void add_default_factory(const std::string& key) {
    add_factory(key, make_prototype<F>(), [](const Conf& c) -> T {
      auto ptr = std::make_unique<F>();
      ptr->from_conf(c);
      return ptr;
    });
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j;
    if (available_.empty()) {
      j["not"] = Json{
          {"description", "no variants available"},
      };
    } else {
      j["oneOf"] = factory_json_schemas();
    }
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    assert(schema_ != nullptr);
    auto factory = c.get<std::string>(factory_key_);
    if (!available_.count(factory)) {
      this->throw_error(c, "unknown factory: {}", factory);
    }

    schema_->validate(c);
  }

  Type make(const Conf& c) const { return deserialize(c); }

  Type deserialize(const Conf& c) const {
    assert(schema_ != nullptr);
    auto factory = c.get<std::string>(factory_key_);
    if (!available_.count(factory)) {
      this->throw_error(c, "unknown factory: {}", factory);
    }

    Conf args;
    if (args_subset_) {
      if (args_key_ != "") {
        if (c.has(args_key_)) {
          args = c.at(args_key_);
        }
      } else {
        args = c;
        args.erase(factory_key_);
      }
    } else {
      args = c;
    }

    return available_.at(factory).func(args);
  }

  Json serialize(const Type& x) const { return x; }

  void from_conf(const Conf& c) override {
    throw std::logic_error("FactoryBase::from_conf() should not be used");
  }

  void to_json(Json& j) const override {
    throw std::logic_error("FactoryBase::to_json() should not be used");
  }

  void reset_ptr() override {
    // No pointer, so nothing to do here.
  }

 protected:
  void reset_schema() { schema_.reset(new Variant(factory_schemas())); }

  std::vector<Box> factory_schemas() const {
    std::vector<Box> out;
    out.reserve(available_.size());
    for (auto& kv : available_) {
      Struct base{
          {factory_key_, make_const_str(kv.first, "name of factory").require()},
      };
      if (args_key_ == "") {
        // FIXME(ben): This branch leads to seg-faults in production code. Not
        // sure if this is because of Struct or Factory x_x
        base.set_properties_from(*kv.second.schema.template as<Struct>());
      } else {
        base.set_property(args_key_, kv.second.schema.clone());
      }
      base.reset_ptr();
      out.emplace_back(std::move(base));
    }
    return out;
  }

  std::vector<Json> factory_json_schemas() const {
    auto schemas = factory_schemas();
    std::vector<Json> out;
    out.reserve(schemas.size());
    for (auto& s : schemas) {
      out.emplace_back(s.json_schema());
    }
    return out;
  }

 protected:
  std::shared_ptr<Variant> schema_;
  std::vector<Box> available_schemas_;
  FactoryMap available_;
  std::string factory_key_{"factory"};
  std::string args_key_{"args"};
  bool args_subset_{true};
};

/**
 * FactoryPointerless is a factory schema that does not refer to any variable
 * instance but is only usable through it's make() method.
 */
template <typename T>
class FactoryPointerless : public FactoryBase<T, FactoryPointerless<T>> {};

/**
 * Factory is a factory schema that extends FactoryPointerless in that it
 * can deserialize into a variable through a pointer.
 *
 * Apart from the added constructors, the interface is otherwise identical
 * to FactoryPointerless.
 */
template <typename T>
class Factory : public FactoryBase<T, Factory<T>> {
 public:  // Types
  using Type = typename FactoryBase<T, Factory<T>>::Type;
  using MakeFunc = typename FactoryBase<T, Factory<T>>::MakeFunc;
  using TypeFactory = typename FactoryBase<T, Factory<T>>::TypeFactory;
  using FactoryMap = typename FactoryBase<T, Factory<T>>::FactoryMap;
  using FactoryPairList = typename FactoryBase<T, Factory<T>>::FactoryPairList;

 public:  // Constructors
  using FactoryBase<T, Factory<T>>::FactoryBase;

  Factory(Type* ptr, std::string&& desc) : FactoryBase<T, Factory<T>>(std::move(desc)), ptr_(ptr) {}

  Factory(Type* ptr, std::string&& desc, FactoryMap&& fs)
      : FactoryBase<T, Factory<T>>(std::move(desc)), ptr_(ptr) {
    for (auto&& f : fs) {
      this->available_.insert(f);
    }
    this->reset_schema();
  }

  Factory(Type* ptr, std::string&& desc, FactoryPairList fs)
      : FactoryBase<T, Factory<T>>(std::move(desc)), ptr_(ptr) {
    for (auto&& f : fs) {
      this->available_.insert(f);
    }
    this->reset_schema();
  }

 public:  // Overrides
  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = this->deserialize(c);
  }

  void to_json(Json& j) const override {
    assert(ptr_ != nullptr);
    j = this->serialize(*ptr_);
  }

  void reset_ptr() override { ptr_ = nullptr; }

 private:
  Type* ptr_{nullptr};
};

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_FACTORY_HPP_
