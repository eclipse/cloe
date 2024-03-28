/*
 * Copyright 2021 Robert Bosch GmbH
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
 * \file fable/examples/contacts/src/main.cpp
 *
 * In this example application, we will define structs to serialize and
 * deserialize JSON for contact persons.
 *
 * We will be using the JSON from this Wikipedia article as our source
 * format:
 *
 *     https://en.wikipedia.org/w/index.php?title=JSON&oldid=1027546414
 *
 * The application should be able to serialize and deserialize to something
 * like this:
 *
 *     {
 *       "firstName": "John",
 *       "lastName": "Smith",
 *       "isAlive": true,
 *       "age": 27,
 *       "address": {
 *         "streetAddress": "21 2nd Street",
 *         "city": "New York",
 *         "state": "NY",
 *         "postalCode": "10021-3100"
 *       },
 *       "phoneNumbers": [
 *         {
 *           "type": "home",
 *           "number": "212 555-1234"
 *         },
 *         {
 *           "type": "office",
 *           "number": "646 555-4567"
 *         }
 *       ],
 *       "children": [],
 *       "spouse": null
 *     }
 *
 * While we don't actually *need* to specify separate structs for the address,
 * it improves readability significantly and makes working with optionals
 * possible.
 */

#include <iostream>  // for std::{cout, cerr}
#include <string>    // for std::string<>
#include <vector>    // for std::vector<>

#include <fmt/format.h>        // for fmt::format
#include <CLI/CLI.hpp>         // for CLI::App
#include <boost/optional.hpp>  // for boost::optional<>

#include <fable/confable.hpp>               // for fable::{Confable, CONFABLE_SCHEMA}
#include <fable/schema.hpp>                 // for fable::{Schema, String}
#include <fable/schema/boost_optional.hpp>  // for fable::{Optional, make_schema}
#include <fable/utility.hpp>                // for fable::{read_conf}

// All structs that are used directly with fable for serialization and
// deserialization need to inherit from fable::Confable and override the
// schema_impl() method.
struct Address : public fable::Confable {
  std::string streetAddress;
  std::string city;
  std::string state;
  std::string postalCode;

  Address() = default;
  Address(std::string street, std::string city, std::string state, std::string code) noexcept
      : streetAddress(std::move(street))
      , city(std::move(city))
      , state(std::move(state))
      , postalCode(std::move(code)) {}

  // The CONFABLE_SCHEMA macro lets us implement schema_impl, and it defines
  // several other helper friend functions to improve interoperability with
  // nlohmann_json to_json and from_json functions.
  CONFABLE_SCHEMA(Address) {
    // Using the fable namespace within this method makes it a lot easier on
    // the eyes. In this namespace the main class available is Schema, which is
    // a convenience wrapper around the type-specific schema implementations.
    //
    // Each type provides special construction methods, like pattern() and
    // not_empty() for string schemas. To access these, we need to use either
    // schema::String or make_schema.
    //
    // See <fable/schema.hpp> and <fable/schema/string.hpp>.
    using namespace fable;
    return Schema{
        {"streetAddress",
         schema::String(&streetAddress, "street and house number").require().not_empty()},
        {"city", schema::String(&city, "city").require().not_empty()},
        {"state", Schema(&state, "state")},
        {"postalCode", schema::String(&postalCode, "postal code").pattern(R"(^[ 0-9/-]*$)")},
    };
  }
};

// Enums are also supported, see <fable/enum.hpp> and <fable/schema/enum.hpp>.
enum class PhoneType {
  Home,
  Mobile,
  Work,
  Other,
};

// This macro creates a bi-directional mapping that Fable uses to serialize
// and deserialize the enum. The parentheses around the initializer list is
// required, due to macro quirks.
//
// Note: normal enums are supported as well, but class enums provide better
// type safety.
//
// Note: clang-format formats this block extremely indented, so we turn off
// clang-format for this section.
//
// clang-format off
ENUM_SERIALIZATION(PhoneType, ({
    {PhoneType::Home, "home"},
    {PhoneType::Mobile, "mobile"},
    {PhoneType::Work, "office"},
    {PhoneType::Other, "other"},
}))
// clang-format on

struct PhoneNumber : public fable::Confable {
  PhoneType type;
  std::string number;

  PhoneNumber() = default;
  PhoneNumber(PhoneType type, std::string number) noexcept : type(type), number(number) {}

  CONFABLE_SCHEMA(PhoneNumber) {
    // Instead of using schema::String or Schema, you can use make_schema,
    // which instantiates the correct schema type which you can then call
    // further construction methods on, such as require(). This is useful
    // when you don't know or care about the underlying schema type.
    using namespace fable;
    return Schema{
        {"type", make_schema(&type, "phone number category").require()},
        {"number", schema::String(&number, "phone number").pattern(R"(^[ +0-9/-]+$)")},
    };
  }
};

// In this final struct, we combine all the above structs together.
//
// Fable lets you choose the data structure that best describes the data,
// without putting many limitations on you.
//
// Currently, std::optional from C++17 is not yet integrated.
struct Contact : public fable::Confable {
  std::string firstName;
  std::string lastName;
  bool isAlive{false};
  boost::optional<uint8_t> age{0};

  boost::optional<Address> address;
  std::vector<PhoneNumber> phoneNumbers;
  std::vector<std::string> children;
  boost::optional<std::string> spouse;

  Contact() = default;
  Contact(
      std::string first, std::string last, bool alive, boost::optional<uint8_t> age) noexcept
      : firstName(std::move(first)), lastName(std::move(last)), isAlive(alive), age(age) {}

  Contact with_address(Address&& addr) && {
    address = std::move(addr);
    return std::move(*this);
  }

  Contact with_phone(PhoneNumber&& tel) && {
    phoneNumbers.emplace_back(std::move(tel));
    return std::move(*this);
  }

  CONFABLE_SCHEMA(Contact) {
    using namespace fable;
    return Schema{
        {"firstName", make_schema(&firstName, "first name of contact").require()},
        {"lastName", make_schema(&lastName, "last name of contact").require()},
        {"isAlive", make_schema(&isAlive, "whether person is alive")},
        {"age", make_schema(&age, "the age of person in years")},
        {"address", make_schema(&address, "the address of person")},
        {"phoneNumbers", make_schema(&phoneNumbers, "the phone numbers of person")},
        {"children", make_schema(&children, "the children of person")},
        {"spouse", make_schema(&spouse, "the spouse of person")},
    };
  }
};

int main(int argc, char** argv) {
  // Parse command line arguments:
  CLI::App app("Fable Contact Example");
  std::string filename;
  app.add_option("-f,--file", filename, "input JSON filepath");
  bool print_example{false};
  bool print_schema{false};
  app.add_flag("--print-example", print_example, "print example data");
  app.add_flag("--print-schema", print_schema, "print data schema");
  CLI11_PARSE(app, argc, argv);

  std::vector<Contact> contacts{
      Contact("John", "Smith", true, 42)
          .with_address({"Generate Road 12", "Nowhere", "NA", "00000"})
          .with_phone({PhoneType::Home, "+1 650 0000 000"}),
      Contact("Jane", "Doe", false, boost::none),
  };

  // If we don't have a Confable, we can create a Schema on the fly and pass
  // in a pointer to the variable it should represent. We should use this
  // schema for serialization/deserialization then.
  fable::Schema schema{&contacts, "address book"};

  if (print_example) {
    std::cout << schema.to_json().dump(2) << std::endl;
    return 0;
  }

  if (print_schema) {
    std::cout << schema.json_schema().dump(2) << std::endl;
    return 0;
  }

  // Ensure the config filename is specified:
  if (!filename.size()) {
    std::cerr << "Error: filename is empty" << std::endl;
    return 1;
  }

  // Load the configuration from the file:
  try {
    schema.from_conf(fable::read_conf(filename));
  } catch (fable::SchemaError& e) {
    fable::pretty_print(e, std::cerr);
    return 1;
  } catch (fable::Error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  std::cout << "NAME                  AGE  ADDRESS" << std::endl;
  std::cout << "--------------------  ---  ----------------------------------" << std::endl;
  for (const auto& c : contacts) {
    std::cout << fmt::format("{:20}  {:3}  {:40}",
                             fmt::format("{} {}", c.firstName, c.lastName),
                             (c.age ? fmt::format("{}", *c.age) : "N/A"),
                             (c.address ? fmt::format("{}, {} {}",
                                                      c.address->streetAddress,
                                                      c.address->postalCode,
                                                      c.address->city)
                                        : "N/A"))
              << std::endl;
  }
}
