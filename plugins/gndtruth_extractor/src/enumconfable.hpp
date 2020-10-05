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
 * \file enumconfable.hpp
 */

#include <cloe/core.hpp>  // for Confable, Schema

#include "bimap.hpp"  // for EnumStringMap

namespace cloe {

template <typename TEnum, typename CRTP>
class EnumStringConfableBase : public Confable {
 public:
  typedef TEnum type1;
  typedef std::string type2;
  type1 value;

  EnumStringConfableBase() : value() {}

  Schema schema_impl() override {
    // return Schema(
    //     "one of [json, json.gz, json.bz2, json.zip, msgpack, msgpack.gz, msgpack.bz2, msgpack.zip]",
    //     Json::value_t::string);
    std::vector<type2> ext;
    boost::copy(EnumStringMap<type1>::right() | boost::adaptors::map_keys, std::back_inserter(ext));
    auto fileExtensions = boost::algorithm::join(ext, ", ");
    return Schema(std::string() + "one of [" + fileExtensions + "]", Json::value_t::string);
  }

  void from_conf(const Conf& c) override {
    // Map configuration type to value type
    auto key = c.get<type2>();
    const auto iter = EnumStringMap<type1>::find(key);
    // if (iter == EnumStringMap<type1>::end<type2>()) {
    if (iter == EnumStringMap<type1>::endRight()) {
      throw ConfError{c, "unknown output type: {}", key};
    }
    value = iter->second;
  }

  void to_json(Json& j) const override {
    j = [this]() {
      // Map value type to configuration type
      const auto iter = EnumStringMap<type1>::find(value);
      //   return (iter != EnumStringMap<type1>::end<type1>()) ? iter->second : "undefined";
      return (iter != EnumStringMap<type1>::endLeft()) ? iter->second : "undefined";
    }();
  }

  CONFABLE_FRIENDS(CRTP)
};  // namespace controller

template <typename TEnum>
class EnumStringConfable : public EnumStringConfableBase<TEnum, EnumStringConfable<TEnum> > {};

}  // namespace cloe
