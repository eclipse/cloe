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
 * \file fable/json/with_std.hpp
 * \see  fable/json.hpp
 *
 * This file contains specializations of `nlohmann::adl_serializer` for std
 * types in order to provide serialization for third-party types.
 */

#pragma once

#include <memory>  // for unique_ptr<>, shared_ptr<>, weak_ptr<>

#include <nlohmann/json.hpp>

/*
 * In order to provide serialization for third-party types, we need to either
 * use their namespace or provide a specialization in that of nlohmann. It is
 * illegal to define anything in the std namespace, so we are left no choice in
 * this regard.
 *
 * See: https://github.com/nlohmann/json
 */
namespace nlohmann {

template <typename T>
struct adl_serializer<std::unique_ptr<T>> {
  static void to_json(json& j, const std::unique_ptr<T>& opt) {
    if (opt) {
      j = *opt;
    } else {
      j = nullptr;
    }
  }
};

template <typename T>
struct adl_serializer<std::shared_ptr<T>> {
  static void to_json(json& j, const std::shared_ptr<T>& opt) {
    if (opt) {
      j = *opt;
    } else {
      j = nullptr;
    }
  }
};

template <typename T>
struct adl_serializer<std::weak_ptr<T>> {
  static void to_json(json& j, const std::weak_ptr<T>& opt) {
    auto ptr = opt.lock();
    if (ptr) {
      j = *ptr;
    } else {
      j = nullptr;
    }
  }
};

}  // namespace nlohmann
