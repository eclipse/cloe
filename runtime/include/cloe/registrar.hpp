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
 * \file cloe/registrar.hpp
 */

#pragma once

#include <list>     // for list<>
#include <memory>   // for unique_ptr<>
#include <string>   // for string
#include <utility>  // for move

#include <cloe/handler.hpp>  // for Handler
#include <cloe/trigger.hpp>  // for ActionFactory, EventFactory, Callback, ...
#include <fable/json.hpp>    // for Json

namespace cloe {

/**
 * The DirectCallback class is a container for triggers that fulfills the
 * Callback interface and is sufficient for 90% of all callback
 * implementations.
 *
 * This can be returned by the Registrar::register_event method and should
 * be stored in the class responsible for triggering the event.
 */
template <typename E, typename... Ctx>
class DirectCallback : public Callback {
 public:
  size_t size() const { return triggers_.size(); }
  bool empty() const { return triggers_.empty(); }

  void emplace(TriggerPtr&& t, const Sync&) override { triggers_.emplace_back(std::move(t)); }
  void to_json(fable::Json& j) const override { j = triggers_; }

  void trigger(const Sync& sync, const Ctx&... args) {
    auto it = triggers_.begin();
    while (it != triggers_.end()) {
      auto& condition = dynamic_cast<E&>((*it)->event());
      if (condition(sync, args...)) {
        if ((*it)->is_sticky()) {
          this->execute((*it)->clone(), sync);
        } else {
          // Remove from trigger list and advance.
          this->execute(std::move(*it), sync);
          it = triggers_.erase(it);
          continue;
        }
      }
      ++it;
    }
  }

 private:
  std::list<TriggerPtr> triggers_;
};

template <typename E, typename... Ctx>
using DirectCallbackPtr = std::shared_ptr<DirectCallback<E, Ctx...>>;

/**
 * A Handler accesses data from an asynchronous content.
 *
 * In order to provide safety and performance, the form of content that the
 * handler accesses must be specified.
 */
enum class HandlerType {
  /**
   * Static content is assumed to always return the exact same data over
   * the course of a simulation.
   */
  STATIC,

  /**
   * Dynamic content means data may be written to as a result of a handler
   * or that there may be data-races.
   *
   * This is also appropriate for handlers that need to make use of certain
   * GET parameters or have a large overhead when creating data.
   */
  DYNAMIC,

  /**
   * Buffered data is like dynamic content, except that a buffer is used
   * to fetch the data; the buffer is updated every cycle.
   *
   * The main restriction is that buffered data types cannot read from
   * the request (since their output is buffered, and fetched via a dummy
   * request). Additionally, buffered handlers cannot write any data.
   */
  BUFFERED,
};

/**
 * Registrar is passed to controllers and simulators to allow them to register
 * action factories as well as web handlers.
 *
 * Note that a Registrar may modify the key or endpoint that you provide.
 */
class Registrar {
 public:
  virtual ~Registrar() noexcept = default;

  /**
   * Register a static web handler that can be given at the endpoint.
   *
   * Currently, the endpoint should be a valid static path, starting with
   * a slash "/". The root represents whatever receives the Registrar, e.g.,
   * the controller or simulator. Any endpoint registered will be prefixed to
   * ensure identifiability.
   */
  virtual void register_static_handler(const std::string& endpoint, Handler) = 0;

  /**
   * Register a web handler that can be accessed at the given endpoint.
   *
   * Currently, the endpoint should be a valid static path, starting with
   * a slash "/". The root represents whatever receives the Registrar, e.g.,
   * the controller or simulator. Any endpoint registered will be prefixed to
   * ensure identifiability.
   */
  virtual void register_api_handler(const std::string& endpoint, HandlerType, Handler) = 0;

  /**
   * Return a new Registrar with the given static handler prefix.
   *
   * The returned object should remain valid even if the object creating it
   * is destroyed.
   */
  virtual std::unique_ptr<Registrar> with_api_prefix(const std::string& prefix) const = 0;

  /**
   * Return a new Registrar with the given dynamic handler prefix.
   *
   * The returned object should remain valid even if the object creating it
   * is destroyed.
   */
  virtual std::unique_ptr<Registrar> with_static_prefix(const std::string& prefix) const = 0;

  /**
   * Return a new Registrar with the given trigger prefix.
   *
   * The returned object should remain valid even if the object creating it
   * is destroyed.
   */
  virtual std::unique_ptr<Registrar> with_trigger_prefix(const std::string& prefix) const = 0;

  /**
   * Register an ActionFactory.
   */
  virtual void register_action(std::unique_ptr<ActionFactory>&&) = 0;

  /**
   * Construct and register an ActionFactory.
   *
   * This replaces what otherwise would be a very common pattern:
   *
   *    r.register_action(std::make_unique<F>(args...));
   *
   * with:
   *
   *    r.register_action<F>(args...);
   */
  template <typename F, typename... Ctx>
  void register_action(Ctx&&... ctx) {
    register_action(std::make_unique<F>(std::forward<Ctx>(ctx)...));
  }

  /**
   * Register an EventFactory.
   *
   * The Callback is shared by the Registrar and the "Originator" that
   * registers the event. The Registrar owner places triggers with the
   * corresponding event into the callback, which the Originator can trigger
   * when the event occurs.
   *
   * \see  cloe/trigger.hpp
   */
  virtual void register_event(std::unique_ptr<EventFactory>&& f, std::shared_ptr<Callback> c) = 0;

  /**
   * Register an EventFactory and return a DirectCallback for storage of
   * events.
   */
  template <typename F, typename... Ctx>
  DirectCallbackPtr<typename F::EventType, Ctx...> register_event(std::unique_ptr<F>&& f) {
    auto callback = std::make_shared<DirectCallback<typename F::EventType, Ctx...>>();
    this->register_event(std::move(f), callback);
    return callback;
  }

  /**
   * Construct and register an EventFactory and return a DirectCallback for
   * storage of events.
   *
   * This replaces what otherwise would be a common pattern:
   *
   *    r.register_event<F, Ctx...>(std::make_unique<F, Args...>(args...));
   *
   * with:
   *
   *    r.register_event<F, Ctx...>(args...);
   */
  template <typename F, typename... Ctx, typename... Args>
  DirectCallbackPtr<typename F::EventType, Ctx...> register_event(Args... args) {
    auto callback = std::make_shared<DirectCallback<typename F::EventType, Ctx...>>();
    this->register_event(std::make_unique<F>(args...), callback);
    return callback;
  }
};

}  // namespace cloe
