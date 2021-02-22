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
 * \file fable/schema/factory_advanced_test.cpp
 * \see  fable/schema/factory.hpp
 * \see  fable/schema/struct.hpp
 */

#include <memory>   // for shared_ptr<>
#include <random>   // for default_random_engine, normal_distribution<>
#include <string>   // for string
#include <utility>  // for move

#include <fable/json/with_std.hpp>   // for to_json
#include <fable/schema.hpp>          // for Confable, Schema
#include <fable/schema/factory.hpp>  // for Factory
#include <fable/utility/gtest.hpp>   // for assert_validate

namespace {

using namespace fable;

using Generator = std::default_random_engine;

template <typename T>
class Distribution : public Confable {
 public:
  virtual ~Distribution() noexcept = default;

  virtual std::string name() const = 0;
  virtual T get(Generator&) const = 0;
  virtual void reset() = 0;

  CONFABLE_FRIENDS(Distribution)
};

template <typename T>
class NormalDistribution : public Distribution<T> {
 public:
  virtual ~NormalDistribution() noexcept = default;

  std::string name() const override { return "normal"; }
  T get(Generator& g) const override { return distribution(g); }

  void reset() override { distribution = std::normal_distribution<T>{mean, std_deviation}; }

  void to_json(Json& j) const override {
    j = Json{
        {"binding", "normal"},
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
      throw std::runtime_error("empty distribution assignment");
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
    this->set_args_key("");
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
    // In case of multiple random number generators, a different seed must
    // be used for each generator (e.g. increment after each rnd_.reset).
    rnd_.reset(distr_default);
    rnd_.reset(seed);
  }

  CONFABLE_SCHEMA(NoiseConf) {
    return Schema{
        {"distribution",
         DistributionFactory(&distr_default, "set distribution binding and arguments")},
    };
  }

 private:
  DistributionPtr distr_default{nullptr};
  Random<double> rnd_ = Random<double>(0, distr_default);
};

}  // anonymous namespace

TEST(fable_schema_factory_advanced, deserialize_distribution) {
  NoiseConf tmp;

  fable::assert_validate(tmp, R"({
    "distribution": {
        "binding": "normal",
        "mean": 1.0,
        "std_deviation": 0.1
    }
  })");
}
