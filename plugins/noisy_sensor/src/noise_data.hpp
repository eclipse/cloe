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
 * \file noise_data.hpp
 */

#pragma once

#include <memory>   // for shared_ptr<>
#include <random>   // for default_random_engine, normal_distribution<>
#include <string>   // for string
#include <utility>  // for move

#include <cloe/component.hpp>        // for Component, ComponentFactory, ...
#include <cloe/core.hpp>             // for Confable, Schema
#include <cloe/entity.hpp>           // for Entity
#include <cloe/simulator.hpp>        // for ModelError
#include <fable/schema/factory.hpp>  // for Factory

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
      {"mean", make_schema(&mean, "mean value of normal distribution")},
      {"std_deviation", make_schema(&std_deviation, "standard deviation of normal distribution")},
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

template <typename T>
class Random {
 public:
  Random(const unsigned long& seed, DistributionPtr dist) : engine_(seed), d(dist) {}

  virtual ~Random() noexcept = default;

  T get() const { return d->get(engine_); }

  void reset(const unsigned long& seed) { engine_ = std::default_random_engine(seed); }

  void reset(DistributionPtr dist) {
    if (dist) {
      d = dist;
    } else {
      throw cloe::ModelError("noisy_sensor: empty distribution assignment.");
    }
  }

 private:
  mutable std::default_random_engine engine_;
  DistributionPtr d{nullptr};
};

class DistributionFactory : public fable::schema::Factory<DistributionPtr> {
 public:
  DistributionFactory(DistributionPtr* ptr, std::string&& desc)
      : fable::schema::Factory<DistributionPtr>(ptr, std::move(desc)) {
    this->set_factory_key("binding");
    this->add_default_factory<NormalDistribution<double>>("normal");
  };
  virtual ~DistributionFactory() = default;
};

class NoiseConf : public Confable {
 public:
  NoiseConf() = default;

  virtual ~NoiseConf() noexcept = default;

  double get() const { return rnd_.get(); }

  virtual void reset(unsigned long seed) {
    rnd_.reset(distr_default);
    rnd_.reset(seed);
    // In case of multiple random number generators, a different seed must
    // be used for each generator (e.g. increment after each rnd_.reset).
  }

  CONFABLE_SCHEMA(NoiseConf) {
    return Schema{
        // clang-format off
        {"distribution", DistributionFactory(&distr_default, "set distribution binding and arguments")},
        // clang-format on
    };
  }

  void to_json(Json& j) const override {
    j = Json{
        {"distribution", distr_default},
    };
  }

 private:
  DistributionPtr distr_default{nullptr};
  Random<double> rnd_ = Random<double>(0, distr_default);
};

struct NoisySensorConf : public Confable {
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

  CONFABLE_SCHEMA(NoisySensorConf) {
    return Schema{
        {"enable", Schema(&enabled, "enable or disable component")},
        {"reuse_seed", Schema(&reuse_seed, "whether to get a new seed on reset")},
        {"seed", Schema(&seed, "set random engine seed (effective on reset)")},
    };
  }

  void to_json(Json& j) const override {
    j = Json{
        {"enable", enabled},
        {"reuse_seed", reuse_seed},
        {"seed", seed},
    };
  }
};

}  // namespace component
}  // namespace cloe