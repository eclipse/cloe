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
 * \file cloe/component/wheel.hpp
 */

#pragma once

#include <fable/json.hpp>  // for to_json

namespace cloe {

struct Wheel {
  /// Rotational angle of wheel around y-axis in [rad].
  double rotation{0.0};

  /// Translative velocity of the wheel in [m/s].
  double velocity{0.0};

  /// Compression of the spring in [m].
  double spring_compression{0.0};

  friend void to_json(fable::Json& j, const Wheel& w) {
    j = fable::Json{
        {"rotation", w.rotation},
        {"velocity", w.velocity},
        {"spring_compression", w.spring_compression},
    };
  }
};

}  // namespace cloe
