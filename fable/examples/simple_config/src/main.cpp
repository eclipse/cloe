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
#include <iostream>  // for std::{cout, cerr}
#include <string>    // for std::string
#include <vector>    // for std::vector<>

#include <fmt/format.h>  // for fmt::format
#include <CLI/CLI.hpp>   // for CLI::App, CLI11_PARSE

#include <fable/confable.hpp>  // for fable::{Conf, Confable, Schema, ...}
#include <fable/utility.hpp>   // for fable::read_conf
#include <fable/schema.hpp>    // for Schema

// A nested configuration structure
struct NestedConfig : public fable::Confable {
  std::vector<std::string> keys;

  CONFABLE_SCHEMA(NestedConfig) {
    return fable::Schema{
        {"keys", fable::Schema(&keys, "List of strings")},
    };
  };
};

// Primary configuration structure
struct ExampleConfig : public fable::Confable {
  bool feature_foo = true;
  bool feature_bar = false;
  NestedConfig nested;  // use the nested structure in the primary one

  // Define the schema to generate the logic required for
  // - Deserialization
  // - Validation
  // - Serialization
  // - Generating the JSON-Schema
  //
  // See fable/schema.hpp for instructions and examples.
  CONFABLE_SCHEMA(ExampleConfig) {
    using namespace fable::schema;
    return Struct{
        {"feature_foo", make_schema(&feature_foo, "Indicates whether feature foo is enabled")},
        {"feature_bar",
         make_schema(&feature_bar, "Indicates whether feature bar is enabled").require()},
        {"nested", make_schema(&nested, "Demonstrates nested data type")},
    };
  };
};

int main(int argc, char **argv) {
  // Prepare the argument parser
  CLI::App app("Fable Example");
  std::string filename;
  app.add_option("-f,--file", filename, "Input JSON file path");

  CLI11_PARSE(app, argc, argv);

  // Ensure the config filename is specified
  if (!filename.size()) {
    std::cerr << "Error: missing file argument" << std::endl;
    std::cerr << "Note: use --help for an overview of options" << std::endl;
    return 1;
  }

  const int indent = 3;
  const int header_width = 80;

  // Load the configuration from the file
  ExampleConfig config;
  try {
    std::cout << fmt::format("Loading config from {}", filename) << std::endl;
    config.from_conf(fable::read_conf(filename));
  } catch (fable::Error& e) {
    std::cerr << fmt::format("\n\n{0:=^{1}}\n", " JSON-Validation ", header_width) << std::endl;
    std::cerr << fmt::format("Error: {}", e.what()) << std::endl;
    return 1;
  }

  // Print some arbitrary values from the configuration
  std::cout << fmt::format("Feature foo is {}enabled!", config.feature_foo ? "" : "not ")
            << std::endl;
  std::cout << fmt::format("Feature bar is {}enabled!", config.feature_bar ? "" : "not ")
            << std::endl;
  std::cout << "Advanced config:" << std::endl;
  for (auto x : config.nested.keys) {
    std::cout << fmt::format("   {}", x) << std::endl;
  }

  // Print the JSON-Schema
  std::cout << fmt::format("\n\n{0:=^{1}}\n", " JSON-Schema ", header_width) << std::endl;
  std::cout << config.schema().json_schema().dump(indent) << std::endl;

  // Print the configuration as JSON
  std::cout << fmt::format("\n\n{0:=^{1}}\n", " JSON-Dump ", header_width) << std::endl;
  std::cout << config.to_json().dump(indent) << std::endl;
}
