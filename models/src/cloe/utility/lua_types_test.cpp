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
 * \file cloe/utility/lua_types_test.cpp
 */

#include <gtest/gtest.h>

#include <cloe/utility/lua_types.hpp>

#include <Eigen/Dense>  // for Eigen

#include <cloe/component/object.hpp>  // for cloe::Object

using DataBroker = cloe::DataBroker;

TEST(lua_types_test, object) {
  //         Test Scenario: positive-test
  // Test Case Description: Implement a vector3d signal and manipulate a member from Lua
  //            Test Steps: 1) Implement a signal
  //                        2) Stimulate the signal from Lua
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: I) The value of the member changed
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};

  // Register all types
  cloe::utility::register_lua_types(db);

  // 1) Implement a signal
  auto gamma = db.implement<cloe::Object>("gamma");

  // bind signals
  db.bind_signal("gamma");
  db.bind("signals");
  // 2) Manipulate a member from Lua
  const auto &code = R"(
    local gamma = signals.gamma
    gamma.type = cloe.types.cloe.Object.Type.Static;
    gamma.classification = cloe.types.cloe.Object.Class.Pedestrian
    signals.gamma = gamma
  )";
  // run lua
  state.open_libraries(sol::lib::base, sol::lib::package);
  state.script(code);
  // verify I
  EXPECT_EQ(gamma->type, cloe::Object::Type::Static);
  EXPECT_EQ(gamma->classification, cloe::Object::Class::Pedestrian);
}

TEST(lua_types_test, vector3d) {
  //         Test Scenario: positive-test
  // Test Case Description: Implement a vector3d signal and manipulate a member from Lua
  //            Test Steps: 1) Implement a signal
  //                        2) Stimulate the signal from Lua
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: I) The value of the member changed
  //                        II) The value-changed event was received
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};

  // Register all types
  cloe::utility::register_lua_types(db);

  // 1) Implement a signal
  auto gamma = db.implement<Eigen::Vector3d>("gamma");
  auto five = db.implement<int>("five");

  // bind signals
  db.bind_signal("gamma");
  db.bind_signal("five");
  db.bind("signals");
  // 2) Manipulate a member from Lua
  const auto &code = R"(
    -- use default-constructor
    local gamma = cloe.types.eigen.Vector3d.new()
    gamma.x = -1
    gamma.y = 1.154431
    gamma.z = 3.1415926
    signals.gamma = gamma

    -- use value-constructor
    local vec = cloe.types.eigen.Vector2i.new(3, 4)

    -- use copy-constructor
    local vec2 = cloe.types.eigen.Vector2i.new(vec)

    -- use member-method
    signals.five = vec2:norm()
  )";
  // run lua
  state.open_libraries(sol::lib::base, sol::lib::package);
  state.script(code);
  // verify I
  EXPECT_EQ(gamma->operator[](0), -1);
  EXPECT_EQ(gamma->operator[](1), 1.154431);
  EXPECT_EQ(gamma->operator[](2), 3.1415926);
  EXPECT_EQ(five, 5);
}
