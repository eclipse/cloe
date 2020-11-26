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
 * \file noisy_object_sensor.cpp
 */

#include <Eigen/Geometry>  // for Isometry3d, Vector3d
#include <memory>          // for shared_ptr<>
#include <random>          // for random_device
#include <string>          // for string

#include <cloe/component.hpp>                // for Component, Json
#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/object.hpp>         // for Object
#include <cloe/component/object_sensor.hpp>  // for ObjectSensor
#include <cloe/conf/action.hpp>              // for actions::ConfigureFactory
#include <cloe/plugin.hpp>                   // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                // for Registrar
#include <cloe/sync.hpp>                     // for Sync
#include <cloe/trigger/set_action.hpp>       // for actions::SetVariableActionFactory
#include "noise_data.hpp"                    // for NoiseData, NoiseConf

namespace cloe {

enum class ObjectField { Translation, Velocity, Acceleration };

// clang-format off
ENUM_SERIALIZATION(ObjectField, ({
    {ObjectField::Translation, "translation"},
    {ObjectField::Velocity, "velocity"},
    {ObjectField::Acceleration, "acceleration"},
}))
// clang-format on

namespace component {

void apply_noise_xy(Eigen::Vector3d* vec, const NoiseConf& noise) {
  vec->x() = vec->x() + noise.get();
  vec->y() = vec->y() + noise.get();
}

void add_noise_translation(Object* obj, const NoiseConf* noise) {
  Eigen::Vector3d transl = obj->pose.translation();
  apply_noise_xy(&transl, *noise);
  obj->pose.translation() = transl;
}

void add_noise_velocity(Object* obj, const NoiseConf* noise) {
  Eigen::Vector3d vel = obj->velocity;
  apply_noise_xy(&vel, *noise);
  obj->velocity = vel;
}

void add_noise_acceleration(Object* obj, const NoiseConf* noise) {
  Eigen::Vector3d accel = obj->acceleration;
  apply_noise_xy(&accel, *noise);
  obj->acceleration = accel;
}

class ObjectNoiseConf : public NoiseConf {
 public:
  ObjectNoiseConf() = default;

  virtual ~ObjectNoiseConf() noexcept = default;

  /**
   * Add noise to target parameter.
   */
  std::function<void(Object*)> apply;

  /**
   * Set the appropriate target function.
   */
  void set_target() {
    using namespace std::placeholders;  // for _1
    switch (target_) {
      case ObjectField::Translation:
        apply = std::bind(add_noise_translation, _1, this);
        break;
      case ObjectField::Velocity:
        apply = std::bind(add_noise_velocity, _1, this);
        break;
      case ObjectField::Acceleration:
        apply = std::bind(add_noise_acceleration, _1, this);
        break;
    }
  }

  CONFABLE_SCHEMA(ObjectNoiseConf) {
    return Schema{
        NoiseConf::schema_impl(),
        fable::schema::PropertyList<fable::schema::Box>{
            // clang-format off
            {"target", Schema(&target_, "data field of the object the noise should be applied to")},
            // clang-format on
        },
    };
  }

  void to_json(Json& j) const override {
    NoiseConf::to_json(j);
    j = Json{
        {"target", target_},
    };
  }

 private:
  ObjectField target_{ObjectField::Translation};
};

struct NoisyObjectSensorConf : public NoisySensorConf {
  /// List of noisy object parameters.
  std::vector<ObjectNoiseConf> noisy_params;

  CONFABLE_SCHEMA(NoisyObjectSensorConf) {
    return Schema{
        NoisySensorConf::schema_impl(),
        fable::schema::PropertyList<fable::schema::Box>{
            // clang-format off
              {"noise", Schema(&noisy_params, "configure noisy parameters")},
            // clang-format on
        },
    };
  }

  void to_json(Json& j) const override {
    NoisySensorConf::to_json(j);
    j = Json{
        {"noise", noisy_params},
    };
  }
};

class NoisyObjectSensor : public ObjectSensor {
 public:
  NoisyObjectSensor(const std::string& name, const NoisyObjectSensorConf& conf,
                    std::shared_ptr<ObjectSensor> obs)
      : ObjectSensor(name), config_(conf), sensor_(obs) {
    reset_random();
  }

  virtual ~NoisyObjectSensor() noexcept = default;

  const Objects& sensed_objects() const override {
    if (cached_) {
      return objects_;
    }
    for (const auto& o : sensor_->sensed_objects()) {
      auto obj = apply_noise(o);
      if (obj) {
        objects_.push_back(obj);
      }
    }
    cached_ = true;
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
    reset_random();
  }

  void abort() override {
    ObjectSensor::abort();
    sensor_->abort();
  }

  void enroll(Registrar& r) override {
    r.register_action(std::make_unique<actions::ConfigureFactory>(
        &config_, "config", "configure noisy object component"));
    r.register_action<actions::SetVariableActionFactory<bool>>(
        "noise_activation", "switch sensor noise on/off", "enable", &config_.enabled);
  }

 protected:
  std::shared_ptr<Object> apply_noise(const std::shared_ptr<Object>& o) const {
    if (!config_.enabled) {
      return o;
    }
    auto obj = std::make_shared<Object>(*o);

    for (auto& np : config_.noisy_params) {
      np.apply(obj.get());
    }
    return obj;
  }

  void reset_random() {
    // Reset the sensor's "master" seed, if applicable.
    unsigned long seed = config_.seed;
    if (seed == 0) {
      std::random_device r;
      do {
        seed = r();
      } while (seed == 0);

      if (config_.reuse_seed) {
        config_.seed = seed;
      }
    }
    for (auto& np : config_.noisy_params) {
      np.set_target();
      np.reset(seed);
      ++seed;
    }
  }

  void clear_cache() {
    objects_.clear();
    cached_ = false;
  }

 private:
  NoisyObjectSensorConf config_;
  std::shared_ptr<ObjectSensor> sensor_;
  mutable bool cached_;
  mutable Objects objects_;
};

DEFINE_COMPONENT_FACTORY(NoisyObjectSensorFactory, NoisyObjectSensorConf, "noisy_object_sensor",
                         "add gaussian noise to object sensor output")

DEFINE_COMPONENT_FACTORY_MAKE(NoisyObjectSensorFactory, NoisyObjectSensor, ObjectSensor)

}  // namespace component
}  // namespace cloe

EXPORT_CLOE_PLUGIN(cloe::component::NoisyObjectSensorFactory)