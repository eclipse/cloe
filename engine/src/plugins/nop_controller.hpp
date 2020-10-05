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
 * \file plugins/nop_controller.hpp
 * \see  plugins/nop_controller.cpp
 */

#pragma once
#ifndef CLOE_CONTROLLER_NOP_HPP_
#define CLOE_CONTROLLER_NOP_HPP_

#include <cloe/controller.hpp>  // for DEFINE_CONTROLLER_FACTORY, Json, ...

namespace cloe {
namespace controller {

struct NopConfiguration : public Confable {
  CONFABLE_FRIENDS(NopConfiguration)
};

/**
 * The NopController is a controller that does nothing, hence the name.
 *
 * This is useful for smoke-test testing.
 */
DEFINE_CONTROLLER_FACTORY(NopFactory, NopConfiguration, "nop", "stand-in that does nothing")

}  // namespace controller
}  // namespace cloe

#endif  // CLOE_CONTROLLER_NOP_HPP_
