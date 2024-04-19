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
 * \file fable/schema/xmagic.hpp
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

#include <map>      // for map<>
#include <string>   // for string
#include <utility>  // for move, forward
#include <vector>   // for vector<>
#include <array>    // for array<>

#include <fable/fable_fwd.hpp>        // for Confable
#include <fable/schema/array.hpp>     // for Array<>
#include <fable/schema/confable.hpp>  // for FromConfable<>
#include <fable/schema/const.hpp>     // for Const<>
#include <fable/schema/map.hpp>       // for Map<>
#include <fable/schema/optional.hpp>  // for Optional<>
#include <fable/schema/vector.hpp>    // for Vector<>

namespace fable::schema {

template <typename T, typename S, std::enable_if_t<std::is_base_of_v<Confable, T>, int>>
auto make_prototype(S&& desc) {
  return FromConfable<T>(std::forward<S>(desc));
}

template <typename T, typename S, std::enable_if_t<!std::is_base_of_v<Confable, T>, int>>
auto make_prototype(S&& desc) {
  return make_schema(static_cast<T*>(nullptr), std::forward<S>(desc));
}

}  // namespace fable::schema
