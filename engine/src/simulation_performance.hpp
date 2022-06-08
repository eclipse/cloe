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
 * \file simulation_performance.hpp
 * \see  simulation_performance.cpp
 */

#pragma once

#include <cstdint>  // for uint64_t
#include <string>   // for string
#include <utility>  // for pair<>, make_pair
#include <vector>   // for vector<>
#include <ostream> // for ostream

#include <cloe/core.hpp>  // for Duration, Json

namespace engine {

/**
 * Store the timing samples for multiple entities for a single step.
 *
 * This represents a line in a CSV file, for example, with each additional
 * column after the first being the entity, in order. For example:
 *
 *    step  minimator  basic  virtue  (cloe_padding)  (cloe_engine)
 *    1     0.3        0.5    1.2     0.0             3.4
 *    2     0.2        0.3    1.1     0.0             3.2
 *    ...
 */
struct TimingSamples {
  uint64_t step;
  std::vector<std::pair<std::string, double>> samples;

 public:
  TimingSamples() = default;
  TimingSamples(uint64_t step) : step(step) {}

  // This implements the suggestion from Scott Meyer's Effective C++ 3rd
  // Edition, Item 25. See also template specialization at bottom of file.
  void swap(TimingSamples& other);

  /**
   * Push back the duration used by the entity.
   *
   * Note that it is valid to re-use the same entity name, iff it follows
   * directly upon itself. (That is: [a, a, b, c, c] is valid, but [a, b, a, c]
   * is not valid.)
   */
  void push_back(std::string&& name, double duration);

  /**
   * Return the total duration the entity used.
   *
   * This method doesn't care about how the entries are ordered.
   * This will return 0 if the entity does not exist.
   */
  double total(const std::string& key) const;

  /**
   * Return the total duration used so far by all entities.
   */
  double total() const;

  /**
   * Return an array of plugin and cloe timing keys.
   *
   * The order reflects the order that the plugins are called.
   */
  std::vector<std::string> keys() const;

  /**
   * Return an array of plugin and cloe timings.
   *
   * This should have the same length as `keys()` and groups values
   * accordingly.
   */
  std::vector<double> values() const;

  friend void to_json(cloe::Json& j, const TimingSamples& s);
};

struct SimulationPerformance {
  std::vector<TimingSamples> steps;

 private:
  TimingSamples buffer;

 public:
  void init_step(uint64_t step) {
    buffer.step = step;
    assert(buffer.samples.is_empty());
  }

  void commit_step(double padding, double cycle) {
    // Names can't contain parenthesis, so we wrap these non-plugin times in
    // parenthesis to disambiguate any plugins that might be named the same.
    push_back("(cloe_padding)", padding);
    push_back("(cloe_engine)", cycle - buffer.total());

    TimingSamples tmp;
    tmp.swap(buffer);
    steps.emplace_back(std::move(tmp));
  }

  void push_back(std::string&& name, double ms) { buffer.push_back(std::move(name), ms); }

  void reset() { steps.clear(); }

  bool is_empty() const { return steps.empty(); }

  /**
   * Return an array of (step, duration) for the given entity.
   */
  std::vector<std::pair<uint64_t, double>> values_for(const std::string& key) const;

  /**
   * Return an array of (step, [duration1, duration2, ...]) for all entities.
   *
   * This can be combined with `keys()` for 1-to-1 mapping, which is useful for
   * tabular output.
   */
  std::vector<std::pair<uint64_t, std::vector<double>>> values() const;

  std::vector<std::string> keys() const {
    assert(!steps.empty());
    return steps[0].keys();
  }

  friend void write_gnuplot(std::ostream& os, const SimulationPerformance& s);
  friend void write_csv(std::ostream& os, const SimulationPerformance& s);
  friend void to_json(cloe::Json& j, const SimulationPerformance& s);
};

}  // namespace engine

namespace std {

// This implements the suggestion from Scott Meyer's Effective C++ 3rd
// Edition, Item 25.
template <>
inline void swap<::engine::TimingSamples>(::engine::TimingSamples& a,
                                          ::engine::TimingSamples& b) noexcept {
  a.swap(b);
}

}  // namespace std
