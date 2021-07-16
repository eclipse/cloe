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
 * \file sumo_vehicle.hpp
 */
#pragma once
#include <iostream>
#include <string>

#include <cloe/models.hpp> // for CloeComponent
#include <cloe/vehicle.hpp>

#include "sumo_nop_component.hpp"

namespace cloe
{
  namespace sumo
  {
    class SumoVehicle : public Vehicle
    {
    public:
      virtual ~SumoVehicle() = default;
      SumoVehicle() = delete;
      SumoVehicle(const std::uint64_t &id, const std::string &name) : Vehicle(id, name)
      {
        this->new_component(new SumoNopComponent(id, "Nop"),
                            cloe::CloeComponent::DEFAULT_LONG_ACTUATOR);
      }
    };
  } // namespace sumo
} // namespace cloe
