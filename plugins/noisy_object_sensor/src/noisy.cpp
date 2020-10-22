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
 * \file noisy.cpp
 *
 * This file also plays around with some ideas for replacing to_json from_json
 * functions with a Confable base class.
 */

#include <memory>  // for shared_ptr<>
#include <random>  // for default_random_engine, normal_distribution<>

#include <cloe/component.hpp>                // for Component, ComponentFactory, ...
#include <cloe/component/object.hpp>         // for Object
#include <cloe/component/object_sensor.hpp>  // forObjectSensor
#include <cloe/conf/action.hpp>              // for actions::ConfigureFactory
#include <cloe/plugin.hpp>                   // for EXPORT_CLOE_PLUGIN
#include <cloe/registrar.hpp>                // for Registrar
#include <cloe/sync.hpp>                     // for Sync
#include <cloe/vehicle.hpp>                  // for Vehicle

namespace cloe {
namespace component {

using Generator = std::default_random_engine;

template <typename T>
class Distribution : public Confable, public Entity {
 public:
  using Entity::Entity;
  virtual ~Distribution() noexcept = default;

  virtual T get(Generator&) const = 0;

  void to_json(Json& j) const override {
    j = Json{
        {"binding", name()},
    };
  }

  virtual void reset() {}

  CONFABLE_FRIENDS(Distribution)
};

template <typename T>
class NormalDistribution : public Distribution<T> {
 public:
  NormalDistribution() : Distribution<T>("normal") {}
  virtual ~NormalDistribution() noexcept = default;

  T get(Generator& g) const override { return distribution(g); }

  void reset() override { distribution = std::normal_distribution<T>{mean, std_deviation}; }

  void to_json(Json& j) const override {
    Distribution<T>::to_json(j);
    j["args"] = Json{
        {"mean", mean},
        {"std_deviation", std_deviation},
    };
  }

  void from_conf(const Conf& c) override {
    Confable::from_conf(c);
    reset();
  }

 protected:
  Schema schema_impl() override {
    // clang-format off
    return Schema{
      {"binding", make_const_schema(this->name(), "identifier of this distribution").require()},
      {"args", Schema{
        {"mean", make_schema(&mean, "mean value of normal distribution")},
        {"std_deviation", make_schema(&std_deviation, "standard deviation of normal distribution")},
      }},
    };
    // clang-format on
  }

 private:
  // Configuration
  double mean = 0.0;
  double std_deviation = 0.1;

  // State
  mutable std::normal_distribution<double> distribution{mean, std_deviation};
};

using DistributionPtr = std::shared_ptr<Distribution<double>>;

template <typename T = double, typename P = std::shared_ptr<Distribution<T>>>
class DistributionSchema : public schema::Base<DistributionSchema<T, P>> {
  using Base = schema::Base<DistributionSchema<T, P>>;
  using Type = P;

  const std::map<std::string, std::function<Distribution<T>*(const Conf& c)>> distributions{
      {"normal",
       [](const Conf& c) {
         auto d = new NormalDistribution<T>();
         if (c.has("args")) {
           d->from_conf(c);
         }
         return d;
       }},
  };

 public:  // Constructors
  DistributionSchema(Type* ptr, std::string&& desc)
      : Base(JsonType::object, std::move(desc))
      , ptr_(ptr)
      , schemas_({
            NormalDistribution<T>().schema(),
        }) {}

 public:  // Special
  const std::vector<Schema>& schemas() const { return schemas_; }
  Json json_schemas() const {
    Json vec = Json::array();
    for (const auto& s : schemas_) {
      vec.push_back(s.json_schema());
    }
    return vec;
  }

 public:  // Overrides
  Json json_schema() const override {
    Json j{
        {"oneOf", json_schemas()},
    };
    this->augment_schema(j);
    return j;
  }

  void validate(const Conf& c) const override {
    size_t valid = 0;
    for (const auto& s : schemas_) {
      if (s.is_valid(c)) {
        valid++;
      }
    }
    if (valid != 1) {
      this->throw_error(c, "require exactly one sub-schema to match");
    }
  }

  void to_json(Json& j) const override { j = serialize(*ptr_); }

  void from_conf(const Conf& c) override {
    assert(ptr_ != nullptr);
    *ptr_ = deserialize(c);
  }

  Json serialize(const Type& x) const { return x; }

  Type deserialize(const Conf& c) const {
    auto binding = c.get<std::string>("binding");
    return Type(distributions.at(binding)(c));
  }

  void reset_ptr() override {
    ptr_ = nullptr;
    for (auto& s : schemas_) {
      s.reset_ptr();
    }
  }

 private:
  Type* ptr_{nullptr};
  std::vector<Schema> schemas_;
};

struct NoisyConf : public Confable {
  /**
   * This flag exists so that an action can modify it at runtime.
   */
  bool enabled = true;

  /**
   * If reuse_seed is true, then in every reset we want to use the same
   * random seed. This is generally the behavior that we want when
   * restarting a simulation, as this preserves the same noise pattern.
   */
  bool reuse_seed = true;

  /**
   * When set to 0, a new random seed is retrieved.
   */
  unsigned long seed = 0;

  /**
   * Which distribution to use.
   */
  DistributionPtr distribution{new NormalDistribution<double>()};

  CONFABLE_SCHEMA(NoisyConf) {
    return Schema{
        {"enable", Schema(&enabled, "enable or disable component")},
        {"reuse_seed", Schema(&reuse_seed, "whether to get a new seed on reset")},
        {"seed", Schema(&seed, "set random engine seed (effective on reset)")},
        {"distribution",
         DistributionSchema<>(&distribution, "set distribution binding and arguments")},
    };
  }

  void to_json(Json& j) const override {
    j = Json{
        {"enable", enabled},
        {"seed", seed},
        {"reuse_seed", reuse_seed},
        {"distribution", distribution},
    };
  }
};  // namespace component

class NoisyObjectSensor : public ObjectSensor {
 public:
  NoisyObjectSensor(const std::string& name, const NoisyConf& conf,
                    std::shared_ptr<ObjectSensor> obs)
      : ObjectSensor(name), config_(conf), engine_(conf.seed), sensor_(obs) {
    reset_random();
  }

  virtual ~NoisyObjectSensor() noexcept = default;

  const Objects& sensed_objects() const override {
    if (!cached_) {
      for (const auto& o : sensor_->sensed_objects()) {
        auto obj = apply_noise(o);
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
    reset_random();
  }

  void abort() override {
    ObjectSensor::abort();
    sensor_->abort();
  }

  void enroll(Registrar& r) override {
    r.register_action(std::make_unique<actions::ConfigureFactory>(&config_, "config",
                                                                  "configure noisy component"));
  }

 protected:
  std::shared_ptr<Object> apply_noise(const std::shared_ptr<Object>& o) const {
    if (config_.enabled) {
      auto obj = std::make_shared<Object>(*o);
      obj->pose.translation().x() = obj->pose.translation().x() + getr();
      obj->pose.translation().y() = obj->pose.translation().y() + getr();
      obj->velocity.x() = obj->velocity.x() + getr();
      obj->velocity.y() = obj->velocity.y() + getr();
      obj->acceleration.x() = obj->acceleration.x() + getr();
      obj->acceleration.y() = obj->acceleration.y() + getr();
      return obj;
    } else {
      return o;
    }
  }

  double getr() const { return config_.distribution->get(engine_); }

  void reset_random() {
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

    engine_ = std::default_random_engine(seed);
  }

  void clear_cache() {
    objects_.clear();
    cached_ = false;
  }

 private:
  // Configuration:
  NoisyConf config_;

  // State:
  mutable std::default_random_engine engine_;
  std::shared_ptr<ObjectSensor> sensor_;
  mutable bool cached_;
  mutable Objects objects_;
};  // namespace component

DEFINE_COMPONENT_FACTORY(NoisyFactory, NoisyConf, "noisy_object_sensor",
                         "add guassian noise to object sensor output")

DEFINE_COMPONENT_FACTORY_MAKE(NoisyFactory, NoisyObjectSensor, ObjectSensor)

}  // namespace component
}  // namespace cloe

// Register factory as plugin entrypoint
EXPORT_CLOE_PLUGIN(cloe::component::NoisyFactory)
