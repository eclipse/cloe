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
 * \file fable/utility/boost_path.hpp
 *
 * This file contains specializations of `nlohmann::adl_serializer` for boost
 * types in order to provide serialization for third-party types.
 */

#pragma once

#include <boost/filesystem/path.hpp>  // for path
#include <nlohmann/json.hpp>          // for adl_serializer<>, json

/*
 * In order to provide serialization for third-party types, we need to either
 * use their namespace or provide a specialization in that of nlohmann. It is
 * illegal to define anything in the std namespace, so we are left no choice in
 * this regard.
 *
 * See: https://github.com/nlohmann/json
 */
namespace nlohmann {

template <>
struct adl_serializer<boost::filesystem::path> {
  static void to_json(json& j, const boost::filesystem::path& p) { j = p.native(); }
  static void from_json(const json& j, boost::filesystem::path& p) { p = j.get<std::string>(); }
};

}  // namespace nlohmann
