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
 * \file hmi_contact.hpp
 *
 * This file defines several HMI (electric) contacts that allow intuitive
 * triggering across multiple cycles.
 *
 * Note: this header file and the associated test are not specific to the
 * basic controller, but they are currently placed here until further use-cases
 * present themselves.
 *
 * Two electrical contacts are modelled in this file:
 *
 * - Switch, which can either be ON or OFF
 * - PushButton, which can trigger a repeating and a release action
 *
 * The classes defined in this header should be used in roughly the following
 * way:
 *
 *     class Controller {
 *       ContactMap hmi_;
 *       bool active{false};
 *       double target_speed{0.0};
 *
 *      public:
 *       Controller() {
 *         // clang-format off
 *         hmi_.add("power", PushButton{
 *           [&]() { active = true; },
 *         });
 *         hmi_.add("plus", PushButton{
 *           [&]() { target_speed += 10.0; },
 *           [&]() { target_speed += 5.0; },
 *         });
 *         hmi_.add("minus", PushButton{
 *           [&]() { target_speed = max(0.0, target_speed - 10.0); },
 *           [&]() { target_speed = max(0.0, target_speed - 5.0); },
 *         });
 *         // clang-format on
 *       }
 *
 *       void init(Registrar* r) {
 *         r->register_factory("hmi", ContactFactory(&hmi_));
 *         r->register_handler("/state/hmi", handler::ToJson<ContactMap>(&hmi_));
 *         r->register_handler("/state/hmi/set", handler::FromConf<ContactMap>(&hmi_));
 *       }
 *
 *       Duration control(const Sync& sync) { hmi_.update(sync.time()); }
 *     };
 *
 */

#pragma once

#include <cmath>        // for fmod
#include <map>          // for map<>
#include <memory>       // for shared_ptr<>
#include <string>       // for string
#include <type_traits>  // for enable_if<>
#include <utility>      // for make_pair<>

#include <boost/algorithm/string.hpp>  // for split, is_any_of

#include <cloe/core.hpp>     // for Confable, Json, Schema, Duration
#include <cloe/trigger.hpp>  // for Trigger, Action, ActionFactory

namespace cloe {
namespace utility {

/**
 * A contact allows electricity to pass through it when it has contact
 * with a source and a drain.
 *
 * This phenomenon is modelled with this superclass, of which
 * Switch and PushButton are the primary subtypes.
 *
 * The function of switches occasionally changes based on the passage of time.
 * What unit is used for time and what effect that has is parameterized via
 * templating.
 */
template <typename D = Duration>
class Contact {
 public:
  explicit Contact(bool active) : active_(active) {}
  virtual ~Contact() = default;

  /**
   * Update the state of a contact.
   *
   * If the contact is not active, it only triggers when true is sent.
   * This is not virtual for performance reasons.
   */
  void update(D time, bool down) {
    // Skip if we are not releasing and we are also not pushing down.
    // This reduces the nominal case (false | false) to a single if instruction.
    if (active_ | down) {
      if (down) {
        contact_down(time);
      } else {
        contact_up(time);
      }
    }
  }

  bool has_contact() const { return active_; }

 protected:
  /**
   * Apply the contact.
   *
   * This corresponds to pushing down on a key or button, or moving a switch
   * to the ON position, and should set active_ to true.
   */
  virtual void contact_down(D time) = 0;

  /**
   * Release the contact.
   *
   * This corresponds to releasing a previous push on a key or button,
   * or moving a switch to the OFF position, and should set active_ to false.
   */
  virtual void contact_up(D time) = 0;

  // Dynamic state
  bool active_{false};
};

/**
 * ContactMap is a convenient way to bundle multiple contacts, as is
 * commonly the case in a Vehicle HMI.
 *
 * All Contacts are added to the matrix, which then takes care of updating
 * the values are the right times and representing their state in JSON.
 */
template <typename D = Duration>
class ContactMap : public Confable {
  struct Button {
    std::shared_ptr<Contact<D>> contact;
    bool state;
  };

  std::map<std::string, Button> buttons_;
  schema::Struct schema_;

 public:
  /**
   * Add a Contact to the matrix.
   */
  void add(const std::string& key, std::shared_ptr<Contact<D>> c) {
    if (buttons_.count(key)) {
      throw std::logic_error("HMI contact '" + key + "' already exists.");
    }
    buttons_.emplace(std::make_pair(key, Button{c, c->has_contact()}));
    schema_.set_property(key, Schema(&(buttons_[key].state), "push " + key + " HMI contact"));
  }

  void add_new(const std::string& key, Contact<D>* c) { add(key, std::shared_ptr<Contact<D>>{c}); }

  /**
   * Update all Contacts in the matrix with the current time.
   *
   * Any from_json set up till this point will then take effect.
   */
  void update(D time) {
    for (auto& elem : buttons_) {
      auto& b = elem.second;
      b.contact->update(time, b.state);
    }
  }

  Schema schema() { return schema_; }

  void to_json(Json& j) const override {
    for (auto& elem : buttons_) {
      j[elem.first] = elem.second.state;
    }
  }

  void from_conf(const Conf& c) override {
    for (auto& elem : buttons_) {
      c.try_from(elem.first, elem.second.state);
    }
  }

  CONFABLE_FRIENDS(ContactMap<D>)
};

template <typename D = Duration>
class UseContact : public Action {
 public:
  UseContact(const std::string& name, ContactMap<D>* m, const Conf& data)
      : Action(name), hmi_(m), data_(data) {}
  ActionPtr clone() const override { return std::make_unique<UseContact<D>>(name(), hmi_, data_); }
  CallbackResult operator()(const Sync&, TriggerRegistrar&) override { from_json(*data_, *hmi_); return CallbackResult::Ok; }

 protected:
  void to_json(Json& j) const override { j = *data_; }

 private:
  ContactMap<D>* hmi_;
  Conf data_;
};

template <typename D = Duration>
class ContactFactory : public ActionFactory {
 public:
  using ActionType = UseContact<D>;
  ContactFactory(ContactMap<D>* m, const std::string& name = "hmi")
      : ActionFactory(name, "connect and disconnect button contacts"), contacts_(m) {}

  TriggerSchema schema() const override {
    return TriggerSchema{
        this->name(),
        this->description(),
        InlineSchema("comma-separated list of buttons to press", "[!]<button>[,...]", true),
        contacts_->schema(),
    };
  }

  ActionPtr make(const Conf& c) const override {
    return std::make_unique<UseContact<D>>(name(), contacts_, c);
  }

  /**
   * Construct a UseContact action from a string.
   *
   * The accepted format is a comma-separated list of buttons to activate,
   * with an optional leading exclamation mark:
   *
   *     basic/hmi=!enable
   *     basic/hmi=resume,plus,enable
   *
   * The order of button names does not matter.
   */
  ActionPtr make(const std::string& s) const override {
    // 1. Split string by comma and ensure there is a valid key in each.
    //    (Note: it is infuriating that this is no easy way to do this in C++.)
    std::vector<std::string> fields;
    boost::split(fields, s, boost::is_any_of(","));

    // 2. Fill JSON with fields set to correct values.
    Json j;
    for (const auto& f : fields) {
      if (f.size() == 0) {
        throw Error("empty entry in comma-separated list");
      }
      bool value = (f[0] == '!' ? false : true);
      std::string key = (value ? f : f.substr(1));
      j[key] = value;
    }

    return make(Conf{j});
  }

 private:
  ContactMap<D>* contacts_;
};

/**
 * Switch simulates a switch contact.
 *
 * Pressing the switch immediately triggers the set function, and unpressing
 * the switch immediately triggers the unset function.
 *
 *    *------------------*
 *    set/push           unset/release
 *
 * In contrast to a PushButton, a switch can start its life in either the ON
 * or the OFF position.
 */
template <typename D = Duration>
class Switch : public Contact<D> {
 public:
  Switch(std::function<void()> set_fn, std::function<void()> unset_fn, bool active)
      : Contact<D>(active), set_func_(set_fn), unset_func_(unset_fn) {}
  virtual ~Switch() = default;

 protected:
  void contact_down(D) override {
    // If we are already active, then ignore
    if (!this->active_) {
      this->active_ = true;
      set_func_();
    }
  }

  void contact_up(D) override {
    assert(this->active_);
    this->active_ = false;
    unset_func_();
  }

 private:
  std::function<void()> set_func_;
  std::function<void()> unset_func_;
};

/**
 * PushButton simulates a push button.
 *
 * Pressing a button generally applies a current to a circuit. This current
 * flows for a time t, which is variable. How often a button is triggered then
 * is subject to the following algorithm:
 *
 *    *------------|-----|-----|-----|-----|---*
 *    push         delay |                     |
 *                       inter-arrival time    release
 *
 * At delay and every inter-arrival time, the repeated function is executed,
 * and at release the repeated function is not performed,
 * or the single function is executed if the delay has not been reached yet.
 *
 * WARNING:
 *   PushButton requires regular updates. On each update it will trigger at
 *   most *once*, so if you update with (0, true) and then again at (10000,
 *   true), it will simply trigger the repeated function once.
 */
template <typename D = Duration>
class PushButton : public Contact<D> {
 public:
  explicit PushButton(std::function<void()> click_fn) : Contact<D>(false), single_func_(click_fn) {}
  PushButton(std::function<void()> click_fn, std::function<void()> repeat_fn)
      : Contact<D>(false), single_func_(click_fn), repeat_func_(repeat_fn) {}
  virtual ~PushButton() = default;

  /**
   * Set the initial delay before a repeated button activation is triggered.
   *
   * Regardless of the delay, on release a "click" is activated.
   */
  void set_delay(D delay) { delay_ = delay; }

  /**
   * Set the interarrival time of repeated button activations.
   *
   * After the first delay, interval amount of time elapses between button
   * activations.
   */
  void set_interval(D interval) { interval_ = interval; }

 protected:
  void contact_down(D time) override {
    if (!this->active_) {
      this->active_ = true;
      last_event_ = time;
      return;
    } else if (!repeat_func_) {
      // If repeat_func_ is not set, than we don't care about the time,
      // and we will never trigger while the button is depressed.
      return;
    }

    if (repeated_) {
      // We've already activated the button at least once.
      if (time - last_event_ > interval_) {
        repeat_func_();
        last_event_ = time;
      }
    } else {
      // We have not activated the button yet.
      if (time - last_event_ > delay_) {
        repeat_func_();
        last_event_ = time;
        repeated_ = true;
      }
    }
  }

  void contact_up(D) override {
    assert(this->active_);
    if (!repeated_) {
      single_func_();
    }
    reset();
  }

  void reset() {
    this->active_ = false;
    repeated_ = false;
    last_event_ = D::zero();
  }

 private:
  std::function<void()> single_func_;
  std::function<void()> repeat_func_;
  D delay_{std::chrono::milliseconds(500)};
  D interval_{std::chrono::milliseconds(250)};

  // Dynamic state
  D last_event_{D::zero()};
  bool repeated_{false};
};

/**
 * The contact namespace contains functions for creating common HMI buttons.
 *
 * In very many cases, you will want to use your own definition. For this, you
 * can see this namespace as a collection of examples, or you can even use
 * some of the helper functions in your own definition. For example, the
 * round_step function may be very useful for other definitions.
 */
namespace contact {

/**
 * Return the value of target incremented up to the next multiple of increment.
 *
 * Restricted to the cases where N is an integral type.
 */
template <typename N>
typename std::enable_if<std::is_integral<N>::value, N>::type round_step(N target, N increment) {
  auto rem = target % increment;
  if (rem != 0) {
    if (increment < 0) {
      return target - rem;
    } else {
      return target + (increment - rem);
    }
  } else {
    return target + increment;
  }
}

/**
 * Return the value of target incremented up to the next multiple of increment.
 *
 * Restricted to the cases where N is an floating type.
 */
template <typename N>
typename std::enable_if<std::is_floating_point<N>::value, N>::type round_step(N target,
                                                                              N increment) {
  auto rem = fmod(target, increment);
  if (rem != 0.0) {
    if (increment < 0) {
      return target - rem;
    } else {
      return target + (increment - rem);
    }
  } else {
    return target + increment;
  }
}

/**
 * Return an ON-OFF switch that reads and writes to *ptr.
 *
 * The switch state can be queries with the has_contact method.
 */
template <typename D = Duration>
Switch<D>* make_switch(bool* ptr) {
  assert(ptr != nullptr);
  return new Switch<D>([ptr]() { *ptr = true; }, [ptr]() { *ptr = false; }, *ptr);
}

/**
 * Return an ON-OFF toggle push-button that reads and writes to *ptr.
 */
template <typename D = Duration>
PushButton<D> make_toggle(bool* ptr) {
  assert(ptr != nullptr);
  return PushButton<D>([ptr]() { *ptr = !(*ptr); });
}

/**
 * Return a push-button that increments *ptr by single every push.
 */
template <typename D = Duration, typename N>
PushButton<D> make_step(N* ptr, N single) {
  assert(ptr != nullptr);
  return PushButton<D>([ptr, single]() { *ptr += single; });
}

/**
 * Return a push-button that increments *ptr by single every short
 * push and by multiple when held down for longer period of time.
 */
template <typename D = Duration, typename N>
PushButton<D> make_step(N* ptr, N single, N multiple) {
  assert(ptr != nullptr);
  return PushButton<D>([ptr, single]() { *ptr += single; },
                       [ptr, multiple]() { *ptr += multiple; });
}

/**
 * Return a push-button that increments *ptr by single every push,
 * rounding up to multiples of single.
 */
template <typename D = Duration, typename N>
PushButton<D> make_round_step(N* ptr, N single) {
  assert(ptr != nullptr);
  assert(single != 0);
  return PushButton<D>([ptr, single]() { *ptr = round_step(*ptr, single); });
}

/**
 * Return a push-button that increments *ptr by single every short
 * push and by multiple when held down for longer period of time,
 * rounding up by single and multiple, respectively.
 */
template <typename D = Duration, typename N>
PushButton<D> make_round_step(N* ptr, N single, N multiple) {
  assert(ptr != nullptr);
  assert(single != 0);
  assert(multiple != 0);
  return PushButton<D>([ptr, single]() { *ptr = round_step(*ptr, single); },
                       [ptr, multiple]() { *ptr = round_step(*ptr, multiple); });
}

/**
 * Return a push-button that increments *ptr by single every push,
 * rounding up to multiples of single.
 *
 * This does not decrement *ptr beyond zero.
 */
template <typename D = Duration, typename N>
PushButton<D> make_round_step_nonnegative(N* ptr, N single) {
  assert(ptr != nullptr);
  assert(single != 0);
  return PushButton<D>([ptr, single]() {
    *ptr = round_step(*ptr, single);
    if (*ptr < 0) {
      *ptr = 0;
    }
  });
}

/**
 * Return a push-button that increments *ptr by single every short
 * push and by multiple when held down for longer period of time,
 * rounding up by single and multiple, respectively.
 *
 * This does not decrement *ptr beyond zero.
 */
template <typename D = Duration, typename N>
PushButton<D> make_round_step_nonnegative(N* ptr, N single, N multiple) {
  assert(ptr != nullptr);
  assert(single != 0);
  assert(multiple != 0);
  return PushButton<D>(
      [ptr, single]() {
        *ptr = round_step(*ptr, single);
        if (*ptr < 0) {
          *ptr = 0;
        }
      },
      [ptr, multiple]() {
        *ptr = round_step(*ptr, multiple);
        if (*ptr < 0) {
          *ptr = 0;
        }
      });
}

}  // namespace contact

}  // namespace utility
}  // namespace cloe
