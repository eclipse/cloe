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
 * \file fable/utility/memory.hpp
 *
 * This file contains specializations of `nlohmann::adl_serializer` for std
 * types.
 *
 * In order to provide serialization for third-party types, we need to either
 * use their namespace or provide a specialization in that of nlohmann. It is
 * illegal to define anything in the std namespace, so we are left no choice in
 * this regard.
 *
 * See: https://github.com/nlohmann/json
 */

#pragma once

#include <optional>  // for optional<>

#include <nlohmann/json.hpp>

namespace nlohmann {

template <typename T>
struct adl_serializer<std::optional<T>> {
  static void to_json(json& j, const std::optional<T>& opt) {
    if (opt) {
      j = *opt;
    } else {
      j = nullptr;
    }
  }

  static void from_json(const json& j, std::optional<T>& opt) {
    if (j.type() == json::value_t::null) {
      opt.reset();
    } else {
      *opt = j.get<T>();
    }
  }
};

}  // namespace nlohmann
