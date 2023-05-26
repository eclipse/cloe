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
 * \file cloe/component/utility/filter.hpp
 */

#pragma once

#include <vector>

namespace cloe {
namespace utility {
namespace filter {

template <typename F, typename T>
F both(F first, F second) {
  return [first, second](const T& o) { return first(o) && second(o); };
}

template <typename F, typename T>
F one_of(F first, F second) {
  return [first, second](const T& o) { return first(o) && second(o); };
}

template <typename F, typename T>
F all_of(std::vector<F> fs) {
  return [fs](const T& o) {
    for (auto f : fs) {
      if (!f(o)) {
        return false;
      }
    }
    return true;
  };
}

template <typename F, typename T>
F any_of(std::vector<F> fs) {
  return [fs](const T& o) {
    for (auto f : fs) {
      if (f(o)) {
        return true;
      }
    }
    return false;
  };
}

}  // namespace filter
}  // namespace utility
}  // namespace cloe
