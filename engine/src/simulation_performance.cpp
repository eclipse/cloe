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
 * \file simulation_performance.cpp
 * \see  simulation_performance.hpp
 */

#include "simulation_performance.hpp"

#include <iostream> // for endl

#include <fmt/format.h> // for join

namespace engine {

void TimingSamples::swap(TimingSamples& other) {
  using std::swap;
  swap(step, other.step);
  swap(samples, other.samples);
}

void TimingSamples::push_back(std::string&& name, double ms) {
  samples.push_back(std::make_pair(name, ms));
}

double TimingSamples::total(const std::string& key) const {
  double sum = 0.0;
  for (const auto& pair : samples) {
    if (pair.first == key) {
      sum += pair.second;
    }
  }
  return sum;
}

double TimingSamples::total() const {
  double sum = 0.0;
  for (const auto& pair : samples) {
    sum += pair.second;
  }
  return sum;
}

std::vector<std::string> TimingSamples::keys() const {
  std::vector<std::string> results;
  if (samples.empty()) {
    return results;
  }

  // Remove duplicate keys (these follow directly after one-another), by
  // appending the buffer when we see a new key. We add the final key at the
  // end then.
  std::string buffer = samples[0].first;
  for (const auto& pair : samples) {
    if (buffer != pair.first) {
      results.push_back(buffer);
      buffer = pair.first;
    }
  }
  results.push_back(buffer);
  return results;
}

std::vector<double> TimingSamples::values() const {
  std::vector<double> results;
  if (samples.empty()) {
    return results;
  }

  // Sum groups of samples belonging to the same entity.
  std::string buffer = samples[0].first;
  double sum = 0.0;
  for (const auto& pair : samples) {
    if (buffer != pair.first) {
      results.push_back(sum);
      buffer = pair.first;
      sum = 0.0;
    }
    sum += pair.second;
  }
  results.push_back(sum);
  return results;
}

void to_json(cloe::Json& j, const TimingSamples& s) {
  j = cloe::Json{
      {"step", s.step},
      {"samples", s.samples},
  };
}

std::vector<std::pair<uint64_t, double>> SimulationPerformance::values_for(const std::string& key) const {
  std::vector<std::pair<uint64_t, double>> results;
  for (const auto& p : steps) {
    results.emplace_back(std::make_pair(p.step, p.total(key)));
  }
  return results;
}

std::vector<std::pair<uint64_t, std::vector<double>>> SimulationPerformance::values() const {
  std::vector<std::pair<uint64_t, std::vector<double>>> results;
  for (const auto& p : steps) {
    results.emplace_back(std::make_pair(p.step, p.values()));
  }
  return results;
}

void to_json(cloe::Json& j, const SimulationPerformance& s) { j = s.steps; }

void write_csv(std::ostream& os, const SimulationPerformance& s) {
  os << fmt::format("step,{}\n", fmt::join(s.keys(), ","));
  for (const auto& x : s.steps) {
    os << fmt::format("{},{}\n", x.step, fmt::join(x.values(), ","));
  }
}

void write_gnuplot(std::ostream& os, const SimulationPerformance& s) {
  os << fmt::format("{}\n", fmt::join(s.keys(), ","));
  for (const auto& x : s.steps) {
    os << fmt::format("{},{}\n", x.step, fmt::join(x.values(), ","));
  }
}

}  // namespace engine
