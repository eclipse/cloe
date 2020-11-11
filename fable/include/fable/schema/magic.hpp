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
 * \file fable/schema/magic.hpp
 * \see  fable/schema.hpp
 * \see  fable/schema_test.cpp
 *
 * This file defines the `make_prototype` functions that can automatically
 * derive the prototype via the `make_schema` functions. It also contains the
 * definitions of constructors that make use of these functions.
 *
 * Unfortunately, all `make_schema` functions need to already be defined at the
 * point of definition of these constructors, which is why they have to be in
 * this file in the first place.
 *
 * So we declare the constructors in their relative class definitions, and
 * we define them here. This file is then included after all other files.
 * And then it works.
 */

#pragma once
#ifndef FABLE_SCHEMA_MAGIC_HPP_
#define FABLE_SCHEMA_MAGIC_HPP_

#include <map>      // for map<>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector<>

#include <fable/schema/array.hpp>     // for Array<>
#include <fable/schema/confable.hpp>  // for FromConfable<>
#include <fable/schema/const.hpp>     // for Const<>
#include <fable/schema/map.hpp>       // for Map<>
#include <fable/schema/optional.hpp>  // for Optional<>

namespace fable {

// Forward declarations:
class Confable;

namespace schema {

template <typename T, typename P>
Array<T, P>::Array(std::vector<T>* ptr, std::string&& desc)
    : Array<T, P>(ptr, make_prototype<T>(), std::move(desc)) {}

template <typename T>
Array<T, decltype(make_prototype<T>())> make_schema(std::vector<T>* ptr, std::string&& desc) {
  return Array<T, decltype(make_prototype<T>())>(ptr, std::move(desc));
}

template <typename T, typename P>
Const<T, P>::Const(const T& constant, std::string&& desc)
    : Const<T, P>(constant, make_prototype<T>(), std::move(desc)) {}

template <typename T>
Const<T, decltype(make_prototype<T>())> make_const_schema(const T& constant, std::string&& desc) {
  return Const<T, decltype(make_prototype<T>())>(constant, std::move(desc));
}

template <typename T, typename P>
Map<T, P>::Map(std::map<std::string, T>* ptr, std::string&& desc)
    : Map<T, P>(ptr, make_prototype<T>(), std::move(desc)) {}

template <typename T>
Map<T, decltype(make_prototype<T>())> make_schema(std::map<std::string, T>* ptr,
                                                  std::string&& desc) {
  return Map<T, decltype(make_prototype<T>())>(ptr, std::move(desc));
}

template <typename T, typename P>
Optional<T, P>::Optional(boost::optional<T>* ptr, std::string&& desc)
    : Optional<T, P>(ptr, make_prototype<T>(), std::move(desc)) {}

template <typename T>
Optional<T, decltype(make_prototype<T>())> make_schema(boost::optional<T>* ptr,
                                                       std::string&& desc) {
  return Optional<T, decltype(make_prototype<T>())>(ptr, std::move(desc));
}

template <typename T, std::enable_if_t<std::is_base_of<Confable, T>::value, int>>
auto make_prototype(std::string&& desc) {
  return FromConfable<T>(std::move(desc));
}

template <typename T, std::enable_if_t<!std::is_base_of<Confable, T>::value, int>>
auto make_prototype(std::string&& desc) {
  return make_schema(static_cast<T*>(nullptr), std::move(desc));
}

}  // namespace schema
}  // namespace fable

#endif  // FABLE_SCHEMA_MAGIC_HPP_
