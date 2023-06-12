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
 * \file cloe/component/actuator.hpp
 */

#pragma once

#include <boost/optional.hpp>  // for optional
#include <cloe/component.hpp>  // for Component
#include <fable/json.hpp>      // for Json
#include <fable/utility/boost_optional.hpp>

namespace cloe {

template <typename T = double>
class Actuator : public Component {
 public:
  using Component::Component;
  virtual ~Actuator() noexcept = default;

  virtual void set(T a) { target_ = a; }
  virtual bool is_set() { return static_cast<bool>(target_); }
  virtual T get() { return *target_; }

  fable::Json active_state() const override { return fable::Json{{"target", target_}}; }

  Duration process(const Sync& sync) override {
    auto t = Component::process(sync);
    target_.reset();
    return t;
  }

  void reset() override {
    Component::reset();
    target_.reset();
  }

 protected:
  boost::optional<T> target_;
};

}  // namespace cloe
