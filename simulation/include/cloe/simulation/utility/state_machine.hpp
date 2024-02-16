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
 * \file utility/state_machine.hpp
 */

#pragma once

#include <cstdint>           // for uint64_t
#include <initializer_list>  // for initializer_list<>
#include <map>               // for map<>
#include <memory>            // for shared_ptr<>
#include <mutex>             // for mutex, lock_guard<>
#include <optional>          // for optional<>
#include <utility>           // for move

#include <cloe/core.hpp>                // for Json
#include <cloe/utility/statistics.hpp>  // for Accumulator
#include <cloe/utility/timer.hpp>       // for DurationTimer<>

#define DEFINE_STATE_STRUCT(MachineType, ContextType, Id, StructName) \
  static constexpr StateId Id = #Id;                                  \
  struct StructName : public State<MachineType, ContextType> {        \
    using State<MachineType, ContextType>::State;                     \
    StateId id() const override { return Id; }                        \
    StateId impl(ContextType& ctx) override;                          \
  }

namespace engine {

using StateId = const char*;

template <typename MachineType, typename ContextType>
class State {
  // The number of times the state machine has entered this state.
  uint64_t calls_;

  // A statistic of the durations that this state has been active.
  cloe::utility::Accumulator timing_ms_;

  // The number of transitions from this state to other states.
  std::map<StateId, uint64_t> transitions_;

  // Pointer to machine that contains this state.
  MachineType* machine_;

 public:
  State(MachineType* ptr) : machine_(ptr) { assert(machine_ != nullptr); }
  virtual ~State() = default;

  /**
   * Return a C-string defining the unique name of this state.
   *
   * It is important that all states be defined globally and statically, so
   * that the pointer to the C-string is always the same.
   */
  virtual StateId id() const = 0;

  /**
   * Return a pointer to the state machine.
   *
   * This is useful to accessing methods on the entire state machine,
   * such as may be used for inserting interrupts.
   */
  MachineType* state_machine() const { return machine_; }

  /**
   * Enter into this state and return the next state that should be executed.
   *
   * If nullptr is returned, then the state machine aborts.
   */
  virtual StateId run(ContextType& ctx) {
    calls_++;
    logger()->trace("Enter state: {}", this->id());
    timer::DurationTimer<> t(
        [&](timer::Milliseconds timing) { this->timing_ms_.push_back(timing.count()); });

    StateId next = impl(ctx);
    if (next != nullptr) {
      transitions_[next]++;
    }
    return next;
  }

  /**
   * Return the logger that should be used for this state.
   */
  virtual cloe::Logger logger() const { return cloe::logger::get("cloe"); }

  friend void to_json(cloe::Json& j, const State<MachineType, ContextType>& s) {
    j = cloe::Json{
        {"id", s.id()},
        {"count", s.calls_},
        {"transitions", s.transitions_},
        {"timing_ms", s.timing_ms_},
    };
  }

 protected:
  /**
   * The implementation of this state behavior.
   */
  virtual StateId impl(ContextType& ctx) = 0;
};

template <typename StateType, typename ContextType>
class StateMachine {
  using StateMap = std::map<StateId, std::shared_ptr<StateType>>;
  StateMap states_{};
  StateId prev_state_{nullptr};
  StateId interrupt_{nullptr};
  std::mutex interrupt_mtx_;

 public:
  StateMachine<StateType, ContextType>() = default;
  virtual ~StateMachine<StateType, ContextType>() = default;

  /**
   * Return the state map.
   */
  const StateMap& states() const { return states_; }

  /**
   * Return the previous state that was run.
   */
  StateId previous_state() const { return prev_state_; }

  /**
   * Return the class that implements the given state ID.
   */
  template <typename StateImpl = StateType>
  std::shared_ptr<StateImpl> get_state(StateId id) {
    return std::dynamic_pointer_cast<StateImpl>(states_.at(id));
  }

  StateId run_state(StateId id, ContextType& ctx) {
    assert(states_.count(id) == 1);
    auto s = states_.at(id);
    try {
      auto next_id = s->run(ctx);
      prev_state_ = id;
      return next_id;
    } catch (...) {
      prev_state_ = id;
      throw;
    }
  }

  void register_state(StateType* s) {
    assert(s != nullptr);
    register_state(std::shared_ptr<StateType>(s));
  }

  void register_state(std::shared_ptr<StateType> s) {
    auto id = s->id();
    assert(states_.count(id) == 0);
    states_[id] = std::move(s);
  }

  void register_states(std::initializer_list<StateType*> init) {
    for (auto& s : init) {
      register_state(s);
    }
  }

  /**
   * Interrupt normal state machine traversal with the following StateId.
   *
   * If one interrupt has already occurred, it is invalid for a further
   * interrupt to occur until the previous one has been processed.
   */
  void push_interrupt(StateId id) {
    logger()->trace("Push interrupt: {}", id);
    std::lock_guard<std::mutex> guard(interrupt_mtx_);
    if (interrupt_ != nullptr) {
      throw std::logic_error{"interrupt queuing is currently not available, already processing: " +
                             std::string(interrupt_)};
    }
    interrupt_ = id;
  }

  std::optional<StateId> pop_interrupt() {
    std::lock_guard<std::mutex> guard(interrupt_mtx_);
    if (interrupt_ == nullptr) {
      return std::nullopt;
    } else {
      auto tmp = interrupt_;
      interrupt_ = nullptr;
      return tmp;
    }
  }

  /**
   * Handle an interrupt.
   *
   * This method will be called between states, with the nominal state that
   * would have occurred being passed along.
   */
  virtual StateId handle_interrupt(StateId nominal, StateId interrupt, ContextType& ctx) = 0;

  /**
   * Return the logger that should be used for this state.
   */
  virtual cloe::Logger logger() const { return cloe::logger::get("cloe"); }
};

}  // namespace engine
