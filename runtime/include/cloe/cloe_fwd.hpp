/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file cloe/cloe_fwd.hpp
 *
 * This file provides forward declarations.
 */

#pragma once

#include <chrono>      // for nanoseconds
#include <functional>  // for function<>
#include <memory>      // for unique_ptr<>, shared_ptr<>

#include <spdlog/fwd.h> // for spdlog namespace
#include <fable/fable_fwd.hpp> // for fable namespace

namespace cloe {

// from core/duration.hpp
using Duration = std::chrono::nanoseconds;

// from core/error.hpp
class Error;
class ConcludedError;

// from core/logger.hpp
using Logger = std::shared_ptr<spdlog::logger>;
using LogLevel = spdlog::level::level_enum;

// from entity.hpp
class Entity;

// from sync.hpp
class Sync;

// from handler.hpp
class Request;
class Response;
enum class ContentType;
enum class RequestMethod;
enum class StatusCode;
using Handler = std::function<void(const Request&, Response&)>;

// from trigger.hpp
class Trigger;
using TriggerPtr = std::unique_ptr<Trigger>;
class TriggerError;
template <typename T>
class TriggerFactory;
class TriggerRegistrar;

class Callback;
enum class Source;

class Event;
using EventPtr = std::unique_ptr<Event>;
using EventFactory = TriggerFactory<Event>;

class Action;
using ActionPtr = std::unique_ptr<Action>;
using ActionFactory = TriggerFactory<Action>;

// from registrar.hpp
enum class HandlerType;
class Registrar;

// from model.hpp
class Model;
class ModelFactory;

// from component.hpp
class Component;
class ComponentFactory;

// from vehicle.hpp
class Vehicle;

// from controller.hpp
class Controller;
class ControllerFactory;

// from simulator.hpp
class Simulator;
class SimulatorFactory;

}  // namespace cloe
