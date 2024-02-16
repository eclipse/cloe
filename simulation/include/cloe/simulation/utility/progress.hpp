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
 * \file utility/progress.hpp
 */

#pragma once

#include <chrono>  // for time_point, now, duration_cast

#include <cloe/core.hpp>  // for Json, Duration

namespace engine {

template <typename T>
cloe::Duration cast_duration(T tp) {
  return std::chrono::duration_cast<cloe::Duration>(tp);
}

class Progress {
 public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::steady_clock::time_point;
  using Duration = cloe::Duration;

  Progress() = default;
  virtual ~Progress() noexcept = default;

  /**
   * Reset the clock to now.
   *
   * This does not alter the current progress bar, because that is anyways
   * overwritten with each update.
   */
  void begin() { beg_ = Clock::now(); }

  /**
   * Set the clock to terminate now.
   *
   * This is better than calling update(1.0) because there are no rounding
   * errors that can occur here. Also, this also stores the termination time
   * which effectively saves the elapsed time.
   */
  void end() {
    end_ = Clock::now();
    eta_ = Duration{0};
    cur_ = 1.0;
  }

  /**
   * Return true if end() was called for this progress.
   */
  bool is_ended() const { return cur_ == 1.0 && eta_ == Duration{0}; }

  /**
   * Update the progress with a value between 0.0 and 1.0.
   */
  void update(double p) {
    if (p == 0.0) {
      // We can't do any predicting when p==0, because we don't have a time
      // interval yet, so just return.
      return;
    }

    assert(p > 0.0 && p <= 1.0);
    auto dur = Clock::now() - beg_;
    assert(cast_duration(dur) > Duration(0));
    prev_ = cur_;
    auto total = Duration(static_cast<long long>(static_cast<double>(dur.count()) / p));
    assert(total >= Duration(0));
    eta_ = total - dur;
    assert(eta_ >= Duration(0));
    cur_ = p;
  }

  /**
   * Update the progress with a value that is corrected to between 0.0 and 1.0.
   */
  void update_safe(double p) {
    if (p < 0.0) {
      p = 0.0;
    } else if (p > 1.0) {
      p = 1.0;
    }
    update(p);
  }

  /**
   * Return the current progress as a percentage between 0.0 and 1.0.
   */
  double percent() const { return cur_; }

  /**
   * Return the expected remaining duration.
   */
  Duration eta() const { return eta_; }

  /**
   * Return the elapsed time since the beginning or the final elapsed time.
   */
  Duration elapsed() const {
    if (cur_ < 1) {
      auto now = Clock::now();
      return cast_duration(now - beg_);
    }
    return cast_duration(end_ - beg_);
  }

  friend void to_json(cloe::Json& j, const Progress& e) {
    j = cloe::Json{
        {"percent", e.percent()},
        {"elapsed", e.elapsed()},
        {"eta", e.eta()},
    };
  }

 private:
  double prev_{0.0};
  double cur_{0.0};
  TimePoint beg_{};
  TimePoint end_{};
  Duration eta_{0};
};

}  // namespace engine
