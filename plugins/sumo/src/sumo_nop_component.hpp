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
 */
/**
 * \file sumo_nop_component.hpp
 */

#pragma once
#include <cloe/component.hpp>
#include <cloe/core.hpp> // for Json
#include <iostream>
#include <string>

namespace cloe
{
  class SumoNopComponent : public cloe::Component
  {
  public:
    SumoNopComponent(const std::uint64_t & /*id*/, const std::string & /*name*/)
        : cloe::Component("nop", "sumo nop component") {}

    virtual Json active_state() const override { return Json{}; }
  };
} // namespace cloe
