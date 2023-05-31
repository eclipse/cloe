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
 * \file fable/enum.hpp
 * \see  fable/schema/enum.hpp
 */

#pragma once

#include <map>          // for map<>
#include <string>       // for string
#include <type_traits>  // for enable_if_t<>, is_enum<>
#include <utility>      // for make_pair, move

#include <fable/json.hpp>

/**
 * Define serialization and deserilization for an enum.
 *
 * This *must* be called in the originating namespace of the enum.
 * If this is not desirable, you can also use FABLE_ENUM_SERIALIZATION.
 *
 * This macro makes it easier to use enum classes and only need to provide a
 * mapping to strings once. Given the type T, it defines the following
 * functions:
 *
 *   void enum_serialization(const std::map<T, std::string>**)
 *   void enum_deserialization(const std::map<std::string, T>**)
 *   void to_json(Json&, const T&)
 *   void from_json(const Json&, T&)
 *
 * These functions are used by the Enum class below to provide easy insertion
 * into Schemas.
 */
#define ENUM_SERIALIZATION(xType, xMap)                                                            \
  inline const std::map<xType, std::string>& enum_serialization(xType) {                           \
    static std::map<xType, std::string> data xMap;                                                 \
    return data;                                                                                   \
  }                                                                                                \
  inline const std::map<std::string, xType>& enum_deserialization(xType) {                         \
    static const std::map<std::string, xType> data = ::fable::invert(enum_serialization(xType())); \
    return data;                                                                                   \
  }                                                                                                \
  inline void to_json(::fable::Json& j, const xType& v) { j = enum_serialization(v).at(v); }       \
  inline void from_json(const ::fable::Json& j, xType& v) {                                        \
    v = enum_deserialization(v).at(j.get<std::string>());                                          \
  }

#define FABLE_ENUM_SERIALIZATION(xType, xMap)                                   \
  namespace fable {                                                             \
  template <>                                                                   \
  struct EnumSerializer<xType> {                                                \
    static const std::map<xType, std::string>& serialization() {                \
      static const std::map<xType, std::string> ser xMap;                       \
      return ser;                                                               \
    }                                                                           \
    static const std::map<std::string, xType>& deserialization() {              \
      static std::map<std::string, xType> de =                                  \
          ::fable::invert(EnumSerializer<xType>::serialization());              \
      return de;                                                                \
    }                                                                           \
  };                                                                            \
  }                                                                             \
  namespace nlohmann {                                                          \
  template <>                                                                   \
  struct adl_serializer<xType> {                                                \
    static void to_json(json& j, const xType& e) { j = ::fable::to_string(e); } \
    static void from_json(const json& j, xType& e) {                            \
      e = ::fable::from_string<xType>(j.get<std::string>());                    \
    }                                                                           \
  };                                                                            \
  }

namespace fable {

/**
 * Invert a map.
 *
 * This requires the map to be an injection; that is, for every element y in Y,
 * there is at most one element x in X, so that (x, y) is in mapping m.
 * It therefore holds: Given {x1, x2} in X and x1 != x2, then m[x1] != m[x2].
 * This unfortunately precludes the possibility of aliases.
 */
template <typename X, typename Y>
std::map<Y, X> invert(const std::map<X, Y>& m) {
  std::map<Y, X> m_prime;
  for (const auto& kv : m) {
    m_prime.insert(std::make_pair(kv.second, kv.first));
  }
  return m_prime;
}

template <typename T>
struct EnumSerializer {
  // Implement:
  // static const std::map<T, std::string>& serialization();

  // Implement:
  // static const std::map<std::string, T>& deserialization();
};

template <typename T>
class has_enum_serializer {
  using one = char;
  using two = struct { char x[2]; };

  template <typename C>
  static one test(decltype(&::fable::EnumSerializer<C>::serialization));
  template <typename C>
  static two test(...);

 public:
  enum { value = sizeof(test<T>(0)) == sizeof(one) };
};

template <typename T>
inline constexpr bool has_enum_serializer_v = has_enum_serializer<T>::value;

template <typename T, bool EnumSerializerPresent>
struct EnumSerializerImpl {};

template <typename T>
struct EnumSerializerImpl<T, true> {
  static const std::map<T, std::string>& serialization_impl() {
    return EnumSerializer<T>::serialization();
  }
  static const std::map<std::string, T>& deserialization_impl() {
    return EnumSerializer<T>::deserialization();
  }
};

template <typename T>
struct EnumSerializerImpl<T, false> {
  static const std::map<T, std::string>& serialization_impl() { return enum_serialization(T()); }
  static const std::map<std::string, T>& deserialization_impl() {
    return enum_deserialization(T());
  }
};

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
const std::map<T, std::string>& enum_serialization() {
  return EnumSerializerImpl<T, has_enum_serializer_v<T>>::serialization_impl();
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
const std::map<std::string, T>& enum_deserialization() {
  return EnumSerializerImpl<T, has_enum_serializer_v<T>>::deserialization_impl();
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
std::string to_string(T x) {
  return enum_serialization<T>().at(x);
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
T from_string(const std::string& s) {
  return enum_deserialization<T>().at(s);
}

}  // namespace fable
