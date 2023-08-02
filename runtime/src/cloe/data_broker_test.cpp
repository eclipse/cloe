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
 * \file cloe/data_broker_test.cpp
 * \see  cloe/data_broker.hpp
 */

#include "cloe/data_broker.hpp"

#include <gtest/gtest.h>

#include <functional>

using cloe::Container;
using cloe::DataBroker;

TEST(databroker, basic_usage_1) {
  //         Test Scenario: <demo>
  // Test Case Description: Connect producer-consumer via a named signal.
  //            Test Steps: 1) Assign a new value to the signal
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1a) value-changed notification arrived
  //                        1b) new value is readable
  DataBroker db;
  // setup container variable
  Container<int> x;
  // setup producer
  x = db.implement<int>("x");
  // setup consumer
  db.signal("x")->subscribe<int>([](const int &) { throw "x changed value"; });
  const auto &x_getter = *db.signal("x")->getter<int>();

  // 1a) test value-changed notification
  EXPECT_THROW({ x = 123; }, const char *);
  // 1b) test value request via obtained getter
  EXPECT_EQ(x_getter(), *x);
  // 1b) test value request
  EXPECT_EQ(db.value<int>("x"), *x);
}

TEST(databroker, basic_usage_2) {
  //         Test Scenario: <demo>
  // Test Case Description: Connect producer-consumer via an alias-name of a signal.
  //            Test Steps: 1) Alias existing signal
  //                        2) Assign a new value to the aliased signal
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1) Signal has an alias
  //                        2a) value-changed notification arrived
  //                        2b) new value is readable
  DataBroker db;
  // setup producer
  auto x = db.implement<int>("xx1xx");
  auto y = db.implement<int>("yy2yy");
  // setup consumer
  db.signal("xx1xx")->subscribe<int>([](const int &) { throw "x changed value"; });

  // ambiguous alias xx1xx -> j
  EXPECT_THROW(auto j = db.alias("(.*\\d.*)", "j"), std::logic_error);
  // distinct alias xx1xx -> k
  auto k = db.alias("(.*1.*)", "k");
  // test the alias
  int value = 123;
  EXPECT_THROW({ db.set_value<int>("k", value); }, const char *);
  EXPECT_EQ(x, value);
}

TEST(databroker, basic_usage_3) {
  //         Test Scenario: <demo>
  // Test Case Description: Connect a generic consumer to named signals of known-datatypes
  //            Test Steps: 1) Read signal value, according to its type
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1a) signal type can be determined
  //                        1b) signal value is readable
  DataBroker db;
  auto char_value = 'u';
  auto int_value = 123;
  auto double_value = 1.23;
  // setup producer
  auto x = db.implement<int>("x");
  auto y = db.implement<char>("y");
  auto z = db.implement<double>("z");
  x = int_value;
  y = char_value;
  z = double_value;
  // test differentiating known signal types
  auto handler = [&](const char *name) {
    // 1a) determine signal
    auto signal = db.signal(name);
    // 1a) determine signal type
    auto type = signal->type();
    if (type == &typeid(char)) {
      // 1b) determine signal value
      EXPECT_EQ(signal->value<char>(), char_value);
    } else if (type == &typeid(int)) {
      // 1b) determine signal value
      EXPECT_EQ(signal->value<int>(), int_value);
    } else {
      throw fmt::format(
          "Datatype of signal '{}' is unsupported. Type: '{}'", signal->name(), type->name());
    }
  };
  handler("x");
  handler("y");
  EXPECT_THROW(handler("z"), std::string);
  EXPECT_THROW(handler("1"), std::out_of_range);
}

TEST(databroker, basic_usage_4) {
  //         Test Scenario: <demo>
  // Test Case Description: Determine from producer side whether consumers are attached to a signal
  //            Test Steps: 1) Determine subscriber-count
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1a) Subscriber-count is readable
  DataBroker db;
  // setup producer
  auto x = db.implement<int>("x");
  auto y = db.implement<int>("y");
  // setup consumer
  db.signal("x")->subscribe<int>([](const int &) { throw "x changed value"; });
  // 1a) test subscriber-count of signals
  EXPECT_EQ(1, x.subscriber_count());
  EXPECT_EQ(true, x.has_subscriber());
  EXPECT_EQ(0, db.signal("y")->subscriber_count());
  EXPECT_EQ(false, db.signal("y")->has_subscriber());
}

TEST(databroker, basic_usage_5) {
  //         Test Scenario: <demo>
  // Test Case Description: Implement accessor-functions and notification mechanism manually
  //            Test Steps: 1) Implement accessor-functions
  //                        2) Implement notification mechanism
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1a) value-changed notification is received
  //                        1b) signal value can be read
  DataBroker db;
  // setup producer
  int x = 123;
  auto x_signal = db.declare<int>("x");
  const auto &x_trigger = x_signal->trigger<int>();
  x_signal->set_getter<int>([&]() -> const int & { return x; });
  x_signal->set_setter<int>([&](const int &value) { x = value; });

  // setup consumer
  db.subscribe<int>("x", [](const int &) { throw "x changed value"; });

  // 1a) test value-changed notification
  EXPECT_THROW({ x_trigger(x); }, const char *);
  // 1b) test value request
  EXPECT_EQ(db.value<int>("x"), x);
}

TEST(databroker, declare) {
  //         Test Scenario: positive-test
  // Test Case Description: Declare & access a signal
  //            Test Steps: 1) Declare a signal
  //                        2) Access a signal
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1) Signal is declared
  //                        2) Signal can be access
  DataBroker db;
  // setup producer
  // 1) Declare
  auto x_container = db.declare<int>("x");
  // 2) Access
  auto x_signal = db.signal("x");
}

TEST(databroker, declare_empty_name) {
  //         Test Scenario: negative-test
  // Test Case Description: Declare a signal with an empty name
  //            Test Steps: 1) Declare a signal
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1) std::invalid_argument
  DataBroker db;
  // 1) Declare with an empty name
  EXPECT_THROW(
      {
        // setup producer
        auto x_container = db.declare<int>("");
      },
      std::invalid_argument);
}

TEST(databroker, declare_duplicate) {
  //         Test Scenario: negative-test
  // Test Case Description: Declare a signal with an duplicate name
  //            Test Steps: 1) Declare a signal
  //                        2) Declare another signal with the same name
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::invalid_argument
  DataBroker db;
  // 1) declare a signal
  auto x_container1 = db.declare<int>("x");
  // 2) declare another signal with the same name
  EXPECT_THROW({ auto x_container2 = db.declare<int>("x"); }, std::out_of_range);
}

TEST(databroker, find_signal) {
  //         Test Scenario: positive-test
  // Test Case Description: Find a signal via its name
  //            Test Steps: 1) Declare a signal
  //                        2) Find the signal via its name
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) Signal was found via its name
  DataBroker db;
  // 1) Declare a signal
  auto x_container = db.declare<int>("x");
  // 2) Find the signal via its name
  auto x_signal = db.signal("x");
}

TEST(databroker, find_signal_empty_name) {
  //         Test Scenario: negative-test
  // Test Case Description: Find a signal via an empty-string
  //            Test Steps: 1) Declare a signal
  //                        2) Try to find the signal via an empty-string
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::invalid_argument
  DataBroker db;
  // 1) Declare a signal
  auto x_container = db.declare<int>("x");
  // 2) Try to find the signal via an empty-string
  EXPECT_THROW({ auto x_signal = db.signal(""); }, std::out_of_range);
}

TEST(databroker, find_signal_wrong_name) {
  //         Test Scenario: negative-test
  // Test Case Description: Find a signal whose name does not exist
  //            Test Steps: 1) Declare a signal
  //                        2) Try to find a signal whose name does not exist
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::invalid_argument
  DataBroker db;
  // 1) Declare a signal
  auto x_container = db.declare<int>("x");
  // 2) Try to find a signal whose name does not exist
  EXPECT_THROW({ auto x_signal = db.signal("y"); }, std::out_of_range);
}

TEST(databroker, value) {
  //         Test Scenario: positive-test
  // Test Case Description: Get the value from the signal by its name
  //            Test Steps: 1) Implement a signal & assign its value
  //                        2) Get the value from the signal by its name
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) Correct value of the signal
  DataBroker db;
  // 1) Implement a signal & assign its value
  auto x_container = db.implement<int>("x");
  auto value = 123;
  x_container = value;
  // 2) Get the value from a signal by its name
  EXPECT_EQ(db.value<int>("x"), value);
}

TEST(databroker, value_not_implemented) {
  //         Test Scenario: negative-test
  // Test Case Description: Get the value from the signal whose getter-function is not implemented
  //            Test Steps: 1) Declare a signal
  //                        2) Get the value from the signal by its name
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: std::logic_error
  DataBroker db;
  // 1) Declare a signal
  db.declare<int>("x");
  // 2) Get the value from a signal by its name
  EXPECT_THROW({ auto x_value = db.value<int>("x"); }, std::logic_error);
}

TEST(databroker, value_incorrect_type) {
  //         Test Scenario: negative-test
  // Test Case Description: Get the value from the signal by using the wrong type
  //            Test Steps: 1) Implement a signal
  //                        2) Get the value from the signal by using a wrong type
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: std::logic_error
  DataBroker db;
  // 1) Implement a signal
  auto x_container = db.implement<int>("x");
  // 2) Get the value from a signal by using a wrong type
  EXPECT_THROW({ auto x_value = db.value<char>("x"); }, std::logic_error);
}

TEST(databroker, getter) {
  //         Test Scenario: positive-test
  // Test Case Description: Get the value of a signal via its getter-function
  //            Test Steps: 1) Implement a signal
  //                        2) Get the getter-function from the signal
  //                        3) Get signal-value from the getter-function
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) getter-function of the signal
  //                        3) correct value of the signal
  DataBroker db;
  // 1) Implement a signal
  auto x_container = db.implement<int>("x");
  auto value = 123;
  x_container = value;
  // 2) Get the getter-function from the signal
  const auto &x_getter = db.getter<int>("x");
  // 3) Get the signal-value from the getter-function
  EXPECT_EQ(x_getter(), value);
}

TEST(databroker, getter_not_implemented) {
  //         Test Scenario: negative-test
  // Test Case Description: Get getter-function from a signal which does not have one
  //            Test Steps: 1) Declare a signal
  //                        2) Get the getter-function from the signal
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::logic_error
  DataBroker db;
  // 1) Declare a signal
  db.declare<int>("x");
  // 2) Get the getter-function from the signal
  EXPECT_THROW({ const auto &x_getter = db.getter<int>("x"); }, std::logic_error);
}

TEST(databroker, getter_incorrect_type) {
  //         Test Scenario: negative-test
  // Test Case Description: Get the getter-function from a signal by using the wrong type
  //            Test Steps: 1) Implement a signal
  //                        2) Get the getter-function from the signal using a wrong type
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::logic_error
  DataBroker db;
  // 1) Implement a signal
  auto x_container = db.implement<int>("x");
  // 2) Get the getter-function from the signal using a wrong type
  EXPECT_THROW({ const auto &x_getter = db.getter<double>("x"); }, std::logic_error);
}

TEST(databroker, set_value) {
  //         Test Scenario: positive-test
  // Test Case Description: Set the value of a signal by its name
  //            Test Steps: 1) Implement a signal
  //                        2) Set the value of the signal by its name
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) Value of the signal set
  DataBroker db;
  // 1) Implement a signal
  auto x_container = db.implement<int>("x");
  // 2) Set the value of the signal by its name
  db.subscribe<int>("x", [](const int &) { throw "x changed value"; });
  EXPECT_THROW({ db.set_value<int>("x", 123); }, const char *);
}

TEST(databroker, set_value_not_implemented) {
  //         Test Scenario: negative-test
  // Test Case Description: Try to set the value of a signal whose setter-function is not implemented
  //            Test Steps: 1) Declare a signal
  //                        2) Set the value of a signal whose setter-function is not implemented
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::logic_error
  DataBroker db;
  // 1) Declare a signal
  auto x_container = db.declare<int>("x");
  // 2) Set the value of a signal whose setter-function is not implemented
  EXPECT_THROW({ db.set_value<int>("x", 123); }, std::logic_error);
}

TEST(databroker, set_value_incorrect_type) {
  //         Test Scenario: negative-test
  // Test Case Description: Try to set the value of a signal by using the wrong datatype
  //            Test Steps: 1) Implement a signal
  //                        2) Set the value of a signal by using the wrong datatype
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::logic_error
  DataBroker db;
  // 1) Implement a signal
  auto x_container = db.implement<int>("x");
  // Set the value of a signal by using the wrong datatype
  EXPECT_THROW({ db.set_value<double>("x", 123); }, std::logic_error);
}

TEST(databroker, setter) {
  //         Test Scenario: positive-test
  // Test Case Description: Get the setter-function of a signal
  //            Test Steps: 1) Implement a signal
  //                        2) Get the setter-function of the signal
  //                        3) Set the signal-value via the setter-function
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) setter-function of the signal
  //                        3) signal-value set
  DataBroker db;
  // 1) Implement a signal
  auto x_container = db.implement<int>("x");
  // 2) Get the setter-function of the signal
  db.subscribe<int>("x", [](const int &) { throw "x changed value"; });
  // 3) Set the signal-value via the setter-function
  auto value = 123;
  const auto &x_setter = db.setter<int>("x");
  EXPECT_THROW({ x_setter(value); }, const char *);
}

TEST(databroker, setter_not_implemented) {
  //         Test Scenario: negative-test
  // Test Case Description: Get the setter-function of a signal whose setter-function is not implemented
  //            Test Steps: 1) Declare a signal
  //                        2) Get the setter-function of the signal whose setter-function is not implemented
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::logic_error
  DataBroker db;
  // 1) Declare a signal
  db.declare<int>("x");
  // 2) Get the setter-function of the signal whose setter-function is not implemented
  EXPECT_THROW({ const auto &x_setter = db.setter<int>("x"); }, std::logic_error);
}

TEST(databroker, setter_incorrect_type) {
  //         Test Scenario: negative-test
  // Test Case Description: Get the setter-function of a signal by using the wrong datatype
  //            Test Steps: 1) Implement a signal
  //                        2) Get the setter-function of the signal by using the wrong datatype
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::logic_error
  DataBroker db;
  // 1) Implement a signal
  auto x_container = db.implement<int>("x");
  // 2) Get the setter-function of the signal by using the wrong datatype
  EXPECT_THROW({ const auto &x_setter = db.setter<double>("x"); }, std::logic_error);
}

TEST(databroker, test_api_type_error_compiler_messages) {
  //         Test Scenario: compiler-error test
  // Test Case Description: Intentionally raises compiler error to (manually) determine the correctness of the implementation
  //            Test Steps: 1) Invoke a function with a datatype which will cause a compiler error
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 1) The compiler throws only the intended static_assert
  DataBroker db;
  int value = 0;

  //NOTE:
  // The basic idea is to allow a manual validation of compiler-errors
  // ==> This code is intentionally commented out
  // The alternatives 'automation' or 'no-test' were not desirable.

  // STEP 1: Define one incompatible datatype
  //
  using incompatible_type = int &;
  // using incompatible_type = void;

  // STEP 2: Define one offending operation
  //
  // cloe::Container<incompatible_type> x;
  // db.declare<incompatible_type>("x");
  // auto x1 = db.implement<incompatible_type>("x");
  // db.subscribe<incompatible_type>("x", [](const int *) {});
  // db.set_value<incompatible_type>("x", value);
  // db.set_value<incompatible_type>("x", &value);
  // db.value<incompatible_type>("x");
  // auto x2 = db.getter<incompatible_type>("x");
  // auto x3 = db.setter<incompatible_type>("x");
  // db.signal("x")->getter<incompatible_type>();
  // db.signal("x")->set_getter<incompatible_type>([]() {
  //   // cleanup compiler output, the code anyhow does not compile
  // #pragma GCC diagnostic push
  // #pragma GCC diagnostic ignored "-Wreturn-local-addr"
  //     int value;
  //     return &value;
  // #pragma GCC diagnostic pop
  //   });
  // db.signal("x")->value<incompatible_type>();
  // auto x4 = db.signal("x")->setter<incompatible_type>();
  // db.signal("x")->set_value<incompatible_type>(&value);
  // auto x5 = db.signal("x")->trigger<incompatible_type>();

  // STEP 3: Rebuild
}

struct CustomData {
  int a;
  double b;
  std::string c;
};

void to_lua(sol::state_view view, CustomData * /* value */) {
  sol::usertype<CustomData> usertype_table = view.new_usertype<CustomData>("CustomData");
  usertype_table["a"] = &CustomData::a;
  usertype_table["b"] = &CustomData::b;
  usertype_table["c"] = &CustomData::c;
}

TEST(databroker, to_lua) {
  //         Test Scenario: negative-test
  // Test Case Description: Get the setter-function of a signal by using the wrong datatype
  //            Test Steps: 1) Implement a signal
  //                        2) Get the setter-function of the signal by using the wrong datatype
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) std::logic_error
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};
  // 1) Implement a signal
  auto gamma = db.implement<CustomData>("gamma");

  db.bind("gamma", "gamma");

  const auto &code = R"(
    gamma.b = 1.154431
	)";
  state.script(code);

  EXPECT_EQ(gamma.value().b, 1.154431);
}
