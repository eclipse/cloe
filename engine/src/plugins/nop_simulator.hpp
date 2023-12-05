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
 * \file plugins/nop_simulator.hpp
 * \see  plugins/nop_simulator.cpp
 */

#pragma once

#include <memory>  // for unique_ptr<>
#include <string>  // for string
#include <vector>  // for vector

#include <cloe/simulator.hpp>  // for Simulator, SimulatorFactory, ...

namespace cloe {
namespace simulator {

struct NopConfiguration : public Confable {
  std::vector<std::string> vehicles{"default"};

  CONFABLE_SCHEMA(NopConfiguration) {
    return Schema{
        {"vehicles", Schema(&vehicles, "list of vehicle names to make available")},
    };
  }

  void to_json(Json& j) const override {
    j = Json{
        {"vehicles", vehicles},
    };
  }
};

DEFINE_SIMULATOR_FACTORY(NopFactory, NopConfiguration, "nop", "stand-in no-operation simulator")

}  // namespace simulator
}  // namespace cloe
