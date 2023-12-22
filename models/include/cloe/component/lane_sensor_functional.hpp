/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file cloe/component/lane_sensor_functional.hpp
 * \see  cloe/component/lane_sensor.hpp
 * \see  cloe/component/lane_boundary.hpp
 *
 * This file provides definitions for common functional idioms with respect
 * to LaneBoundaries and LaneBoundarySensors.
 */

#pragma once

#include <functional>  // for function
#include <memory>      // for shared_ptr<>
#include <string>      // for string

#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/component/lane_sensor.hpp>    // for LaneBoundarySensor

namespace cloe {

/**
 * LaneBoundaryFilter shall return true for any LaneBoundary that should be yielded,
 * and false for every LaneBoundary that should be skipped.
 */
using LaneBoundaryFilter = std::function<bool(const LaneBoundary&)>;

/**
 * LaneBoundarySensorFilter filters lane boundaries from an LaneBoundarySensor, and can be used in
 * place of the original LaneBoundarySensor.
 *
 * This class can be used in a very functional way, and the use of C++11
 * lambdas is highly highly recommended!
 *
 * Warning: Do not rely on volatile state that can change within a step for the
 * filter function.  This LaneBoundarySensor filter class caches the resulting vector
 * of filtered lane boundaries till clear_cache is called.
 *
 * Example:
 *
 * ```
 * using namespace cloe;
 * LaneBoundaryFilter my_super_filter;
 * LaneBoundarySensorInterface my_lbs;
 * my_lbs = LaneBoundarySensorFilter(my_lbs, my_super_filter);
 * ```
 *
 * You can also use the filters in cloe::utility::filter to combine Filters:
 *
 * ```
 * using filter = cloe::utility::filter;
 * my_lbs = LaneBoundarySensorFilter(my_lbs, filter::And(my_super_filter, [](const LaneBoundary& o) {
 *   if (o.id < 64) {
 *     if IsStatic(o) {
 *       // ...
 *       return true;
 *     }
 *   }
 *   return false;
 * }));
 * ```
 */
class LaneBoundarySensorFilter : public LaneBoundarySensor {
 public:
  LaneBoundarySensorFilter(std::shared_ptr<LaneBoundarySensor> lbs, LaneBoundaryFilter f)
      : LaneBoundarySensor("lane_sensor_filter"), sensor_(lbs), filter_func_(f) {}

  virtual ~LaneBoundarySensorFilter() noexcept = default;

  /**
   * Return all sensed lane boundaries that the LaneBoundaryFilter returns true for.
   */
  const LaneBoundaries& sensed_lane_boundaries() const override {
    if (!cached_) {
      for (const auto& kv : sensor_->sensed_lane_boundaries()) {
        auto lb = kv.second;
        auto id = kv.first;
        if (filter_func_(lb)) {
          lbs_.insert(std::pair<int, LaneBoundary>(id, lb));
        }
      }
      cached_ = true;
    }
    return lbs_;
  }

  const Frustum& frustum() const override { return sensor_->frustum(); }

  const Eigen::Isometry3d& mount_pose() const override { return sensor_->mount_pose(); }

  /**
   * Process the underlying sensor and clear the cache.
   *
   * We could process and create the filtered list of lane boundaries now, but we can
   * also delay it (lazy computation) and only do it when absolutely necessary.
   * This comes at the minor cost of checking whether cached_ is true every
   * time sensed_lane_boundaries() is called.
   */
  Duration process(const Sync& sync) override {
    // This currently shouldn't do anything, but this class acts as a prototype
    // for How It Should Be Done.
    Duration t = LaneBoundarySensor::process(sync);
    if (t < sync.time()) {
      return t;
    }

    // Process the underlying sensor and clear the cache.
    t = sensor_->process(sync);
    clear_cache();
    return t;
  }

  void reset() override {
    LaneBoundarySensor::reset();
    sensor_->reset();
    clear_cache();
  }

  void abort() override {
    LaneBoundarySensor::abort();
    sensor_->abort();
  }

 protected:
  void clear_cache() {
    lbs_.clear();
    cached_ = false;
  }

 private:
  mutable bool cached_;
  mutable LaneBoundaries lbs_;
  std::shared_ptr<LaneBoundarySensor> sensor_;
  LaneBoundaryFilter filter_func_;
};

}  // namespace cloe
