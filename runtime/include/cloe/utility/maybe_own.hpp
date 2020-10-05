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
 * \file cloe/utility/maybe_own.hpp
 */

#pragma once
#ifndef CLOE_UTILITY_MAYBE_OWN_HPP_
#define CLOE_UTILITY_MAYBE_OWN_HPP_

#include <memory>

namespace cloe {
namespace utility {

template <typename T>
class Deleter {
  bool owned_;

 public:
  Deleter() : owned_(false) {}
  Deleter(bool owned) : owned_(owned) {}  // NOLINT
  void operator()(T* p) {
    if (owned_) {
      delete p;
    }
  }
};

/**
 * maybe_own is a unique_ptr that sometimes has a no-op deleter,
 * depending on how the Deleter class is defined.
 *
 * The best way to use this class is by using the C++11 initializer lists:
 * ```
 * maybe_own<MyClass> mp;
 * MyClass c;
 * mp = {&c, false};  // not-owned
 *
 * MyClass* cp = new MyClass();
 * mp = {cp, true};  // owned, c not deleted
 *
 * mp = {new MyClass(), true};  // owned, cp now deleted
 * ```
 */
template <typename T>
using maybe_own = std::unique_ptr<T, Deleter<T>>;

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_MAYBE_OWN_HPP_
