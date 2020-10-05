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
 * \file cloe/component/object_sensor_functional.hpp
 * \see  cloe/component/object_sensor.hpp
 * \see  cloe/component/object.hpp
 *
 * This file provides definitions for common functional idioms with respect
 * to Objects and ObjectSensors.
 */

#pragma once
#ifndef CLOE_COMPONENT_OBJECT_SENSOR_FUNCTIONAL_HPP_
#define CLOE_COMPONENT_OBJECT_SENSOR_FUNCTIONAL_HPP_

#include <functional>  // for function
#include <memory>      // for shared_ptr<>
#include <string>      // for string

#include <cloe/component/object.hpp>         // for Object
#include <cloe/component/object_sensor.hpp>  // for ObjectSensor

namespace cloe {

/**
 * ObjectFilter shall return true for any Object that should be yielded,
 * and false for every Object that should be skipped.
 */
using ObjectFilter = std::function<bool(const Object&)>;

/**
 * ObjectFilterMap may map and filter objects at the same time.
 *
 * - If it yields the object without changes, it may pass it on.
 * - If it yields the object with changes, it should create a clone of the
 *   Object with `std::make_shared` first, and then make the changes.
 * - If it should skip the object, it should return nullptr.
 */
using ObjectFilterMap = std::function<std::shared_ptr<Object>(const std::shared_ptr<Object>&)>;

/**
 * ObjectSensorFilter filters objects from an ObjectSensor, and can be used in
 * place of the original ObjectSensor.
 *
 * This class can be used in a very functional way, and the use of C++11
 * lambdas is highly highly recommended!
 *
 * Warning: Do not rely on volatile state that can change within a step for the
 * filter function.  This ObjectSensor filter class caches the resulting vector
 * of filtered objects till clear_cache is called.
 *
 * Example:
 *
 * ```
 * using namespace cloe;
 * ObjectFilter my_super_filter;
 * ObjectSensorInterface my_obs;
 * my_obs = ObjectSensorFilter(my_obs, my_super_filter);
 * ```
 *
 * You can also use the filters in cloe::utility::filter to combine Filters:
 *
 * ```
 * using filter = cloe::utility::filter;
 * my_obs = ObjectSensorFilter(my_obs, filter::And(my_super_filter, [](const Object& o) {
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
class ObjectSensorFilter : public ObjectSensor {
 public:
  ObjectSensorFilter(std::shared_ptr<ObjectSensor> obs, ObjectFilter f)
      : ObjectSensor("object_sensor_filter"), sensor_(obs), filter_func_(f) {}

  virtual ~ObjectSensorFilter() noexcept = default;

  /**
   * Return all sensed objects that the ObjectFilter returns true for.
   */
  const Objects& sensed_objects() const override {
    if (!cached_) {
      for (auto o : sensor_->sensed_objects()) {
        if (filter_func_(*o)) {
          objects_.push_back(o);
        }
      }
      cached_ = true;
    }
    return objects_;
  }

  const Frustum& frustum() const override { return sensor_->frustum(); }

  const Eigen::Isometry3d& mount_pose() const override { return sensor_->mount_pose(); }

  /**
   * Process the underlying sensor and clear the cache.
   *
   * We could process and create the filtered list of objects now, but we can
   * also delay it (lazy computation) and only do it when absolutely necessary.
   * This comes at the minor cost of checking whether cached_ is true every
   * time sensed_objects() is called.
   */
  Duration process(const Sync& sync) override {
    // This currently shouldn't do anything, but this class acts as a prototype
    // for How It Should Be Done.
    Duration t = ObjectSensor::process(sync);
    if (t < sync.time()) {
      return t;
    }

    // Process the underlying sensor and clear the cache.
    t = sensor_->process(sync);
    clear_cache();
    return t;
  }

  void reset() override {
    ObjectSensor::reset();
    sensor_->reset();
    clear_cache();
  }

  void abort() override {
    ObjectSensor::abort();
    sensor_->abort();
  }

 protected:
  void clear_cache() {
    objects_.clear();
    cached_ = false;
  }

 private:
  mutable bool cached_;
  mutable Objects objects_;
  std::shared_ptr<ObjectSensor> sensor_;
  ObjectFilter filter_func_;
};

/**
 * ObjectSensorFilterMap filters and maps objects from an ObjectSensor, and can
 * be used in place of the original ObjectSensor.
 *
 * This class can be used in a very functional way, and the use of C++11
 * lambdas is highly highly recommended!
 *
 * Warning: Do not rely on volatile state that can change within a step for the
 * filter function.  This ObjectSensor filter class caches the resulting vector
 * of filtered objects till clear_cache is called.
 */
class ObjectSensorFilterMap : public ObjectSensor {
 public:
  ObjectSensorFilterMap(const std::string& name, std::shared_ptr<ObjectSensor> obs,
                        ObjectFilterMap f)
      : ObjectSensor(name), sensor_(obs), map_func_(f) {}

  virtual ~ObjectSensorFilterMap() noexcept = default;

  const Objects& sensed_objects() const override {
    if (!cached_) {
      for (auto o : sensor_->sensed_objects()) {
        auto obj = map_func_(o);
        if (obj) {
          objects_.push_back(obj);
        }
      }
      cached_ = true;
    }
    return objects_;
  }

  const Frustum& frustum() const override { return sensor_->frustum(); }

  const Eigen::Isometry3d& mount_pose() const override { return sensor_->mount_pose(); }

  /**
   * Process the underlying sensor and clear the cache.
   *
   * We could process and create the filtered list of objects now, but we can
   * also delay it (lazy computation) and only do it when absolutely necessary.
   * This comes at the minor cost of checking whether cached_ is true every
   * time sensed_objects() is called.
   */
  Duration process(const Sync& sync) override {
    // This currently shouldn't do anything, but this class acts as a prototype
    // for How It Should Be Done.
    Duration t = ObjectSensor::process(sync);
    if (t < sync.time()) {
      return t;
    }

    // Process the underlying sensor and clear the cache.
    t = sensor_->process(sync);
    clear_cache();
    return t;
  }

  void reset() override {
    ObjectSensor::reset();
    sensor_->reset();
    clear_cache();
  }

  void abort() override {
    ObjectSensor::abort();
    sensor_->abort();
  }

 protected:
  virtual void clear_cache() {
    objects_.clear();
    cached_ = false;
  }

 protected:
  mutable bool cached_;
  mutable Objects objects_;
  std::shared_ptr<ObjectSensor> sensor_;
  ObjectFilterMap map_func_;
};

}  // namespace cloe

#endif  // CLOE_COMPONENT_OBJECT_SENSOR_FUNCTIONAL_HPP_
