/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file fable/utility/number.hpp
 */

#pragma once

#include <cstdint> // for int8_t, ...
#include <limits>  // for numeric_limits

namespace fable {

/**
 * Return true if casting value to target type is safe.
 *
 * Safe means the specific value provided will not have its sign or value changed
 * by the conversion.
 */
template <typename T, typename S>
constexpr bool is_cast_safe(S value) {
  if (static_cast<S>(static_cast<T>(value)) != value) {
    // Ensure no narrowing is occurring.
    return false;
  }

  if constexpr (std::numeric_limits<T>::is_signed != std::numeric_limits<S>::is_signed) {
    // Mismatch in signedness can go wrong in two ways that aren't caught above.
    if constexpr (std::numeric_limits<S>::is_signed) {
      // Check value will not underflow.
      return value >= 0;
    } else {
      // Check value will not overflow.
      return static_cast<unsigned long long>(value) <=
             static_cast<unsigned long long>(std::numeric_limits<T>::max());
    }
  }

  return true;
}

/**
 * Encode the string name of a type as a static member.
 * This is used for error messages.
 *
 * Example:
 *
 *     std::cout << typeinfo<int8_t>::name << std::endl;
 */
template <typename T>
struct typeinfo {
  static const constexpr char* name = "unknown";
};

// clang-format off
template <> struct typeinfo<bool>     { static const constexpr char* name = "bool"; };
template <> struct typeinfo<int8_t>   { static const constexpr char* name = "int8_t"; };
template <> struct typeinfo<int16_t>  { static const constexpr char* name = "int16_t"; };
template <> struct typeinfo<int32_t>  { static const constexpr char* name = "int32_t"; };
template <> struct typeinfo<int64_t>  { static const constexpr char* name = "int64_t"; };
template <> struct typeinfo<uint8_t>  { static const constexpr char* name = "uint8_t"; };
template <> struct typeinfo<uint16_t> { static const constexpr char* name = "uint16_t"; };
template <> struct typeinfo<uint32_t> { static const constexpr char* name = "uint32_t"; };
template <> struct typeinfo<uint64_t> { static const constexpr char* name = "uint64_t"; };
// clang-format on

}  // namespace fable
