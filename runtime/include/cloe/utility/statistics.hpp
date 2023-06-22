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
 * \file cloe/utility/statistics.hpp
 * \see  cloe/utility/statistics_test.cpp
 */

#pragma once

#include <cmath>   // for sqrt
#include <map>     // for map<>
#include <string>  // for string

#include <fable/json.hpp>  // for Json

namespace cloe {
namespace utility {

template <typename T>
class Pie {
 public:
  Pie() {}

  void reset() {
    n_ = 0;
    map_.clear();
  }

  void push_back(T key) {
    n_++;
    map_[key]++;
  }

  uint64_t count() const { return n_; }

  uint64_t count(T key) const { return map_.count(key) ? map_.at(key) : 0; }

  T mode() const {
    T mode;
    uint64_t max = 0;
    for (auto& kv : map_) {
      if (kv.second > max) {
        mode = kv.first;
        max = kv.second;
      }
    }
    return mode;
  }

  double proportion(T key) const {
    return static_cast<double>(count(key)) / static_cast<double>(n_);
  }

  std::map<T, double> proportions() const {
    const double nd = static_cast<double>(n_);
    std::map<T, double> prop;
    for (auto& kv : map_) {
      prop[kv.first] = static_cast<double>(kv.second) / nd;
    }
    return prop;
  }

  // TODO(ben): Maybe specialize this for types that are directly supported by to_json.
  //            Alternatively, specialize on the nlohmann end of the story.
  friend void to_json(fable::Json& j, const Pie<T>& pie) {
    j = fable::Json{{"count", pie.n_}};

    // Distribution of values:
    std::map<std::string, uint64_t> dist;
    for (auto& kv : pie.map_) {
      dist[to_string(kv.first)] = kv.second;
    }
    j["distribution"] = dist;

    // Proportions of values:
    std::map<std::string, double> prop;
    for (auto& kv : pie.proportions()) {
      prop[to_string(kv.first)] = kv.second;
    }
    j["proportions"] = prop;
  }

 private:
  uint64_t n_{0};
  std::map<T, uint64_t> map_;
};

/**
 * The Accumulator class calculates running statistics for a series of values.
 *
 * - The storage requirements are constant.
 * - Each value that is given to the accumulator updates the statistics, but is itself discarded.
 * - Default constructed value is valid.
 */
class Accumulator {
 public:
  Accumulator() { reset(); }

  /**
   * Reset all values to their defaults.
   *
   * - This can be useful if you want to reset the statistics after a warm-up phase.
   */
  void reset() {
    n_ = 0;
    mean_ = 0;
    var_ = 0;
    max_ = -INFINITY;
    min_ = INFINITY;
  }

  /**
   * Give the accumulator a new value to incorporate.
   *
   * - The various statistics are updated internally.
   */
  void push_back(double x) {
    if (n_ == 0) {
      n_ = 1;
      mean_ = x;
      var_ = 0;
      min_ = x;
      max_ = x;
      return;
    }

    n_++;
    if (max_ < x) {
      max_ = x;
    }
    if (min_ > x) {
      min_ = x;
    }
    double m = mean_ + (x - mean_) / static_cast<double>(n_);
    var_ = var_ + (x - mean_) * (x - m);
    mean_ = m;
  }

  /**
   * Number of values accumulated.
   */
  uint64_t count() const { return n_; }

  /**
   * Maximum value encountered.
   */
  double max() const { return max_; }

  /**
   * Minimum value encountered.
   */
  double min() const { return min_; }

  /**
   * Mean value across all values.
   */
  double mean() const { return mean_; }

  /**
   * Sample or population (default) variance.
   *
   * The population variance is for approximating the true mean of a population of values,
   * of which the series is a sample. The sample variance pertains only to the sample.
   */
  double variance(bool sample = false) const {
    if (n_ <= 1) {
      if (n_ == 0) {
        return NAN;
      }
      return 0;
    }
    if (sample) {
      return var_ / static_cast<double>(n_ - 1);
    } else {
      return var_ / static_cast<double>(n_);
    }
  }

  /**
   * Sample or population (default) standard deviation.
   *
   * - Is derived from the variance, hence the same rules apply for whether to use sample or
   *   population mode.
   */
  double std_dev(bool sample = false) const { return std::sqrt(variance(sample)); }

  /**
   * Writes the JSON representation into j.
   */
  friend void to_json(fable::Json& j, const Accumulator& a) {
    j = fable::Json{
        {"count", a.n_},
        {"min", a.min_},
        {"max", a.max_},
        {"mean", a.mean_},
        {"variance", a.variance()},
        {"std_deviation", a.std_dev()},
        {"sample_variance", a.variance(true)},
        {"sample_std_deviation", a.std_dev(true)},
    };
  }

 private:
  uint64_t n_;
  double mean_;
  double var_;
  double max_;
  double min_;
};

}  // namespace utility
}  // namespace cloe
