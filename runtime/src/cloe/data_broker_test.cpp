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
      throw fmt::format("Datatype of signal '{}' is unsupported. Type: '{}'", signal->name(),
                        type->name());
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

TEST(databroker, container_default_ctor) {
  //         Test Scenario: positive-test
  // Test Case Description: Use default ctor of container
  //            Test Steps: 1) Create container-variable
  //                        2) Implement signal & assign container to the container-variable
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: <no issues>

  DataBroker db;
  // 1) Implement container
  Container<int> x_container;
  // 2) Implement a signal
  x_container = db.implement<int>("x");
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

TEST(databroker, to_lua_1) {
  //         Test Scenario: positive-test
  // Test Case Description: Implement a custom datatype and manipulate a member from Lua
  //            Test Steps: 1) Implement a signal
  //                        2) Stimulate the signal from Lua
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: I) The value of the member changed
  //                        II) The value-changed event was received
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};
  // 1) Implement a signal
  auto gamma = db.implement<double>("gamma");
  auto gamma2 = 2.71828;
  db.subscribe<double>("gamma", [&](const double &value) { gamma2 = value; });

  auto euler = db.declare<double>("euler");
  auto euler2 = 0.0;
  euler->set_setter<double>([&](const double &value) { euler2 = value; });

  const char *special_name = "  special.characters  ";
  auto special = db.implement<double>(special_name);

  // bind signals
  db.bind_signal("gamma");
  db.bind_signal("euler");
  db.bind_signal(special_name);

  db.bind("signals");
  // 2) Manipulate a member from Lua
  const auto &code = R"(
    signals.gamma = 1.154431
    signals.euler = 2.71828
    signals["  special.characters  "] = -1.0
  )";
  // run lua
  state.open_libraries(sol::lib::base, sol::lib::package);
  state.script(code);
  // verify I
  EXPECT_EQ(gamma, 1.154431);
  EXPECT_EQ(special, -1);
  // verify II
  EXPECT_EQ(gamma2, 1.154431);
}

/**
 * Model for arbitrary custom classes
 */
struct CustomData {
  int a{0};
  double b{0};
  std::string c{};
  double d{0};
  double get_d() { return d; }
  void set_d(double value) { d = value; }
};

/**
 * Mandatory ADL function to bind the CustomData type to the Lua-VM
 */
void to_lua(sol::state_view view, CustomData * /* value */) {
  sol::usertype<CustomData> usertype_table = view.new_usertype<CustomData>("CustomData");
  usertype_table["a"] = &CustomData::a;
  usertype_table["b"] = &CustomData::b;
  usertype_table["c"] = &CustomData::c;
  usertype_table["d"] = sol::property(&CustomData::get_d, &CustomData::set_d);
}

TEST(databroker, to_lua_2) {
  //         Test Scenario: positive-test
  // Test Case Description: Implement a custom datatype and manipulate a member from Lua
  //            Test Steps: 1) Implement a signal with a custom datatype
  //                        2) Stimulate a member from Lua
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: 2) The value of the member changed
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};
  // 1) Implement a signal
  auto euler = db.implement<CustomData>("euler");
  auto euler2 = 0.0;
  db.subscribe<CustomData>("euler", [&](const CustomData &value) { euler2 = value.b; });
  auto gamma = db.implement<CustomData>("gamma");
  // bind signals
  db.bind_signal("euler");
  db.bind_signal("gamma");
  db.bind("signals");
  // 2) Manipulate a member from Lua
  const auto &code = R"(
    -- This is not what you think
    signals.euler.b = 2.71828
    signals.euler.d = 2.71828

    -- This is what you want to do:
    local gamma = CustomData.new()
    gamma.b = 1.154431
    gamma.d = 1.154431
    signals.gamma = gamma
	)";
  // run lua
  state.open_libraries(sol::lib::base, sol::lib::package);
  state.script(code);
  // verify
  // EXPECT_EQ(euler.value().b, 2.71828); // This would require that a std::ref is bound to the Lua-VM
  // EXPECT_EQ(euler.value().d, 2.71828); //
  EXPECT_EQ(euler.value().b, 0.0);
  EXPECT_EQ(euler.value().d, 0.0);
  EXPECT_EQ(euler2, 0);  // value-changed event does not work with references :(
  EXPECT_EQ(gamma.value().b, 1.154431);
  EXPECT_EQ(gamma.value().d, 1.154431);
}

/**
  * Model for arbitrary custom enumerator
  */
enum CustomEnum : int { Normal = 1, Exception = -1, Unexpected = -2 };

/**
 * Mandatory ADL function to bind the CustomData type to the Lua-VM
 */
void to_lua(sol::state_view view, CustomEnum * /* value */) {
  // clang-format off
  view.new_enum("CustomEnum"
    ,"Normal", CustomEnum::Normal
    ,"Exception", CustomEnum::Exception
    ,"Unexpected", CustomEnum::Unexpected
  );
  // clang-format on
}

/**
 * C++ exception handler which is preprocessing potential exceptions
 */
int my_exception_handler(lua_State *L, sol::optional<const std::exception &> maybe_exception,
                         sol::string_view description) {
  std::cout << "An exception occurred ";
  if (maybe_exception) {
    std::cout << "description straight from the exception: ";
    const std::exception &ex = *maybe_exception;
    std::cout << ex.what();
  } else {
    std::cout << "description from the description parameter: ";
    std::cout.write(description.data(), static_cast<std::streamsize>(description.size()));
  }
  std::cout << std::endl;
  // you must push 1 element onto the stack to be
  // transported through as the error object in Lua
  // note that Lua -- and 99.5% of all Lua users and libraries -- expects a string
  // so we push a single string (in our case, the description of the error)
  return sol::stack::push(L, description);
}

TEST(databroker, to_lua_3) {
  //         Test Scenario: positive-test
  // Test Case Description: Implement a custom datatype and manipulate a member from Lua
  //            Test Steps: 1) Implement a signal
  //                        2) Stimulate the signal from Lua
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: I) The value of the member changed
  //                        II) The value-changed event was received
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};
  // 1) Implement a signal
  auto tau = db.implement<CustomEnum>("tau");
  tau = CustomEnum::Exception;

  db.subscribe<CustomEnum>("tau", [&](const CustomEnum &value) {
    switch (value) {
      case CustomEnum::Normal:
        // its ok
        break;
      case CustomEnum::Exception:
        throw "This is an exception";
        break;
      default:
        throw std::runtime_error("This is not good");
    }
  });

  // bind signals
  db.bind_signal("tau");
  db.bind("signals");
  // 2) Manipulate a member from Lua
  const auto &code = R"(
    -- Custom error-handler function
    function myerrorhandler( err )
      if (err == "This is an exception") then
        print("I knew it would happen, it's fine.")
      else
        print("This is the end my friend.")
        print(debug.traceback())
        return "not good"
      end
    end

    print("tau: " .. tostring(signals.tau))
    -- If you are confident that
    -- such an assignments works
    -- just do it
    signals.tau = CustomEnum.Normal
    print("tau: " .. tostring(signals.tau))

    -- If you are less confident that
    -- such an assignment works
    -- use an error-handler
    status, result = xpcall( function()
      signals.tau = CustomEnum.Exception
    end , myerrorhandler )
    if (result ~= nil) then
      return
    end

    -- If you are less confident that
    -- such an assignment works
    -- use an error-handler
    status, result = xpcall( function()
      signals.tau = CustomEnum.Unexpected
    end , myerrorhandler )
    if (result ~= nil) then
      return
    end

    print("tau: " .. tostring(signals.tau))
  )";
  // run lua
  state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::debug, sol::lib::os);
  state.set_exception_handler(&my_exception_handler);
  state.script(code);

  EXPECT_EQ(tau, CustomEnum::Unexpected);
}

template <typename T, typename Tag>
struct Quantity {
  T value_;

  Quantity() : value_{} {}
  Quantity(const double &value) : value_{value} {}
};

struct km_tag {};

using km = Quantity<double, km_tag>;

km operator""_km(long double value) { return km{static_cast<double>(value)}; }

/**
 * Mandatory ADL function to bind the CustomData type to the Lua-VM
 */
void to_lua(sol::state_view view, km * /* value */) {
  // clang-format off
  sol::usertype<km> usertype_table = view.new_usertype<km>("km",sol::constructors<km(), km(const double&)>{} );
  usertype_table["value_"] = &km::value_;
  // clang-format on
}

TEST(databroker, to_lua_4) {
  //         Test Scenario: positive-test
  // Test Case Description: Implement a custom datatype and manipulate a member from Lua
  //            Test Steps: 1) Implement a signal
  //                        2) Stimulate the signal from Lua
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: I) The value of the member changed
  //                        II) The value-changed event was received
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};
  // 1) Implement a signal
  auto tau = db.implement<km>("tau");

  // bind signals
  db.bind_signal("tau");
  db.bind("signals");
  // 2) Manipulate a member from Lua
  const auto &code = R"(
    local tau = km.new(1.2)
    signals.tau = tau
  )";
  // run lua
  state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::debug, sol::lib::os);
  state.script(code);

  EXPECT_EQ(tau->value_, 1.2);
}

TEST(databroker, to_lua_5) {
  //         Test Scenario: positive-test
  // Test Case Description: Implement a custom datatype and manipulate a member from Lua
  //            Test Steps: 1) Implement a signal
  //                        2) Stimulate the signal from Lua
  //          Prerequisite: -
  //             Test Data: -
  //       Expected Result: I) The value of the member changed
  //                        II) The value-changed event was received
  sol::state state;
  sol::state_view view(state);
  DataBroker db{view};
  // 1) Implement a signal
  auto optional1 = db.implement<std::optional<int>>("optional1");
  auto optional2 = db.implement<std::optional<int>>("optional2");
  auto optional3 = db.implement<std::optional<int>>("optional3");
  optional1 = 0;
  optional2 = std::optional<int>();
  optional3 = 0;

  // bind signals
  db.bind_signal("optional1");
  db.bind_signal("optional2");
  db.bind_signal("optional3");
  db.bind("signals");
  // 2) Manipulate a member from Lua
  const auto &code = R"(
    if (signals.optional1 == 0) then
      signals.optional1 = 1
    end
    signals.optional1 = 1
    if (signals.optional2 == nil) then
      signals.optional3 = nil
    end
  )";
  // run lua
  state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::debug, sol::lib::os);
  state.script(code);

  EXPECT_EQ(optional1->has_value(), true);
  EXPECT_EQ(optional1->value(), 1);
  EXPECT_EQ(optional2->has_value(), false);
  EXPECT_EQ(optional3->has_value(), false);
}

//         Test Scenario: positive-test
// Test Case Description: Tag an object in various ways with predefined datatypes and read the value of the tags
//            Test Steps: 1) Tag the object
//          Prerequisite: -
//             Test Data: -
//       Expected Result: I) The value of the tags is unchanged
//                        II) Only the actually tags are available
struct important_tag : cloe::MetaInformation::Tag<int> {};
struct paramount_tag : cloe::MetaInformation::Tag<std::string> {};
struct principal_tag : cloe::MetaInformation::Tag<double> {};
struct frivolous_tag : cloe::MetaInformation::Tag<void> {};

TEST(metainformations, metainformation_1) {
  cloe::MetaInformation metainformations;
  // step 1
  metainformations.add<important_tag>(1);
  metainformations.add<paramount_tag>("Hello World");
  metainformations.add<principal_tag>(3.1415);
  // expected I
  EXPECT_EQ(*metainformations.get<important_tag>(), 1);
  EXPECT_EQ(*metainformations.get<paramount_tag>(), "Hello World");
  EXPECT_EQ(*metainformations.get<principal_tag>(), 3.1415);
  // expected II
  EXPECT_FALSE(metainformations.get<frivolous_tag>());
}

//         Test Scenario: positive-test
// Test Case Description: Define tag-type and value-type at once
//            Test Steps: 1) Tag the object
//          Prerequisite: -
//             Test Data: -
//       Expected Result: I) The value of the tag is unchanged
struct prime_tag : cloe::MetaInformation::Tag<prime_tag> {
  bool x;
  std::string y;
  double z;

  bool operator==(const prime_tag &rhs) const {
    return ((x == rhs.x) && (y == rhs.y) && (z == rhs.z));
  }
};

TEST(metainformations, metainformation_2) {
  cloe::MetaInformation metainformations;
  // step -1
  prime_tag metainformation;
  metainformation.x = 1;
  metainformation.y = "Hello World";
  metainformation.z = 3.1415;
  metainformations.add(metainformation);
  // expectation I
  EXPECT_EQ(*metainformations.get<prime_tag>(), metainformation);
}

//         Test Scenario: positive-test
// Test Case Description: Tag a signal with metadata
//            Test Steps: 1) Create a signal
//                        2) Tag the signal
//          Prerequisite: -
//             Test Data: -
//       Expected Result: I) The value of the tag is unchanged

TEST(metainformations, metainformation_3) {
  DataBroker db;
  // step 1
  auto signal = db.declare<int>("x");
  // step 2
  signal->add_metadata<important_tag>(1);
  signal->add_metadata<paramount_tag>("Hello World");
  signal->add_metadata<principal_tag>(3.1415);
  signal->add_metadata<frivolous_tag>();
  // expected I
  EXPECT_EQ(*signal->metadata<important_tag>(), 1);
  EXPECT_EQ(*signal->metadata<paramount_tag>(), "Hello World");
  EXPECT_EQ(*signal->metadata<principal_tag>(), 3.1415);
  EXPECT_TRUE(signal->metadata<frivolous_tag>());
}

//         Test Scenario: positive-test
// Test Case Description: Tag a signal with metadata
//            Test Steps: 1) Create a signal
//                        2) Tag the signal
//          Prerequisite: -
//             Test Data: -
//       Expected Result: I) The value of the tag is unchanged

struct tag_data {
  bool x;
  std::string y;
  double z;

  bool operator==(const tag_data &rhs) const {
    return std::tie(x, y, z) == std::tie(rhs.x, rhs.y, rhs.z);
  }
};

struct shared_tag_1 : cloe::MetaInformation::Tag<const tag_data &> {};
struct shared_tag_2 : cloe::MetaInformation::Tag<std::reference_wrapper<const tag_data>> {};

TEST(metainformations, metainformation_4) {
  cloe::MetaInformation metainformations;
  // step -1
  tag_data metainformation;
  metainformation.x = 1;
  metainformation.y = "Hello World";
  metainformation.z = 3.1415;
  //metainformations.add<shared_tag_1>(metainformation);
  metainformations.add<shared_tag_2>(metainformation);
  // expectation I
  //EXPECT_EQ(*metainformations.get<shared_tag_1>(), metainformation);
  EXPECT_EQ(metainformations.get<shared_tag_2>()->get(), metainformation);
}
