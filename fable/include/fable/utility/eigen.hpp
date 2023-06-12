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
 * \file fable/utility/eigen.hpp
 * \see  fable/json.hpp
 *
 * This file contains specializations of `nlohmann::adl_serializer` for Eigen
 * types in order to provide JSON serialization.
 */

#pragma once

#include <Eigen/Geometry>     // for Vector3d, Quaterniond, Isometry3d
#include <nlohmann/json.hpp>  // for adl_serializer<>, json

namespace nlohmann {

template <>
struct adl_serializer<Eigen::Vector3d> {
  static void to_json(json& j, const Eigen::Vector3d& v) {
    j = json{
        {"x", v.x()},
        {"y", v.y()},
        {"z", v.z()},
    };
  }

  static void from_json(const json& j, Eigen::Vector3d& v) {
    v.x() = j["x"].get<double>();
    v.y() = j["y"].get<double>();
    v.z() = j["z"].get<double>();
  }
};

template <>
struct adl_serializer<Eigen::Quaterniond> {
  static void to_json(json& j, const Eigen::Quaterniond& q) {
    j = json{
        {"w", q.w()},
        {"x", q.x()},
        {"y", q.y()},
        {"z", q.z()},
    };
  }

  static void from_json(const json& j, Eigen::Quaterniond& q) {
    q.w() = j["w"].get<double>();
    q.x() = j["x"].get<double>();
    q.y() = j["y"].get<double>();
    q.z() = j["z"].get<double>();
  }
};

template <>
struct adl_serializer<Eigen::Isometry3d> {
  static void to_json(json& j, const Eigen::Isometry3d& o) {
    Eigen::Vector3d trans = o.translation();
    j = json{
        {"translation", trans},
        {"rotation", Eigen::Quaterniond(o.rotation())},
    };
  }

  static void from_json(const json& j, Eigen::Isometry3d& o) {
    o.setIdentity();
    o.linear() = j["rotation"].get<Eigen::Quaterniond>().matrix();
    o.translation() = j["translation"].get<Eigen::Vector3d>();
  }
};

}  // namespace nlohmann
