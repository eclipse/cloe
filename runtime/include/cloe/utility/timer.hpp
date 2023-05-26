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
 * \file cloe/utility/timer.hpp
 *
 * This file defines types useful for measuring times and doing something with
 * the measurement.
 *
 * Example:
 * ```cpp
 * void some_func() {
 *     ScopeTimer t([]() {
 *       // Do something with the time.
 *     });
 *
 *     // Do whatever...
 * }
 * ```
 */

#pragma once

#include <chrono>      // for now, duration, ...
#include <functional>  // for functional<>

namespace timer {

using Milliseconds = std::chrono::duration<double, std::milli>;
using TimePoint = std::chrono::high_resolution_clock::time_point;

class ScopeTimer {
 public:
  explicit ScopeTimer(std::function<void(TimePoint, TimePoint)> fn) : fn_(fn) {
    start_ = std::chrono::high_resolution_clock::now();
  }

  ScopeTimer(const ScopeTimer&) = delete;
  ScopeTimer& operator=(const ScopeTimer&) = delete;
  ScopeTimer(ScopeTimer&&) = delete;
  ScopeTimer const& operator=(ScopeTimer&&) = delete;

  ~ScopeTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    fn_(start_, end);
  }

 private:
  TimePoint start_;
  std::function<void(TimePoint, TimePoint)> fn_;
};

template <typename P = Milliseconds>
class DurationTimer {
 public:
  DurationTimer() { this->reset(); }
  explicit DurationTimer(std::function<void(P)> fn) : fn_(fn) { this->reset(); }

  DurationTimer(const DurationTimer&) = delete;
  DurationTimer& operator=(const DurationTimer&) = delete;
  DurationTimer(DurationTimer&&) = delete;
  DurationTimer const& operator=(DurationTimer&&) = delete;

  ~DurationTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    if (fn_) {
      fn_(std::chrono::duration_cast<P>(end - start_));
    }
  }

  P reset() {
    auto previous_start = start_;
    start_ = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<P>(start_ - previous_start);
  }

  P elapsed() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<P>(now - start_);
  }

 private:
  TimePoint start_;
  std::function<void(P)> fn_;
};

}  // namespace timer
