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
 * \file cloe/entity.hpp
 * \see  cloe/entity.cpp
 */

#pragma once

#include <string>  // for string

#include <cloe/core.hpp>  // for Error, Logger
#include <fable/json.hpp> // for Json

namespace cloe {

/**
 * InvalidNameError is thrown when a name is invalid.
 */
class InvalidNameError : public Error {
 public:
  explicit InvalidNameError(std::string name)
      : Error("name is invalid: " + name), name_(std::move(name)) {}
  virtual ~InvalidNameError() noexcept = default;

  const std::string& name() const { return name_; }

 private:
  std::string name_;
};

/**
 * An Entity is the base class for all named objects.
 */
class Entity {
 public:
  explicit Entity(std::string name) {
    set_name(std::move(name));
    set_description("");
  }

  Entity(std::string name, std::string desc) {
    set_name(std::move(name));
    set_description(std::move(desc));
  }

  virtual ~Entity() noexcept = default;

  /**
   * Return the name of the Entity.
   */
  const std::string& name() const { return name_; }

  /**
   * Set the name of the Entity.
   *
   * The name must conform to the following regular expression:
   *
   *   ^[a-zA-Z_][a-zA-Z0-9_]*(/[a-zA-Z_][a-zA-Z0-9_]*)*$
   *
   * That is, the following are proper names:
   *
   *   basic/hmi
   *   simulation/stop
   *   start
   *   _/strange_but_0k
   */
  void set_name(std::string name);

  /**
   * Return the optional description of the Entity.
   *
   * If there is no description, the empty string is returned.
   */
  const std::string& description() const { return desc_; }

  /**
   * Set the free-form description of the entity.
   */
  void set_description(std::string desc) { desc_ = std::move(desc); }

 protected:
  /**
   * Return the Logger that the manager should use.
   */
  virtual Logger logger() const { return logger::get(name()); }

  // Note: moving the definition from the inline into this declaration here
  // will result in cloe/trigger.hpp not compiling because:
  //
  //   no matching function for call to 'to_json(fable::Json, const Entity&)'
  //
  // I have no idea why it works in many other instances, but not here. If you,
  // my dear reader, know why, please open a pull request!
  friend void to_json(fable::Json& j, const Entity& e);

 protected:
  std::string name_;
  std::string desc_;
};

/**
 * Return JSON representation of an Entity.
 */
inline void to_json(fable::Json& j, const Entity& e) {
  j["name"] = e.name();
  if (!e.desc_.empty()) {
    j["description"] = e.description();
  }
}

}  // namespace cloe
