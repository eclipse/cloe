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
 * \file plugins/nop_controller.cpp
 * \see  plugins/nop_controller.hpp
 */

#include "cloe/stacklib/plugins/nop_controller.hpp"

#include <memory>  // for unique_ptr<>, make_unique<>

#include <cloe/sync.hpp>  // for Sync

namespace cloe {
namespace controller {

class NopController : public Controller {
 public:
  using Controller::Controller;
  NopController() : Controller("nop") {}
  virtual ~NopController() noexcept = default;

  void reset() override {
    // Nothing to do here.
  }

  void abort() override {
    // Nothing to do here.
  }

  Duration process(const Sync& sync) override { return sync.time(); }
};

std::unique_ptr<Controller> NopFactory::make(const Conf&) const {
  return std::make_unique<NopController>(this->name());
}

}  // namespace controller
}  // namespace cloe
