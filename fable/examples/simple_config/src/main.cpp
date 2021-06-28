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
#include <iostream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <CLI/CLI.hpp>
#include <fable/conf.hpp>
#include <fable/confable.hpp>
#include <fable/utility.hpp>

// A nested configuration structure
struct AdvancedConfig : public fable::Confable {
  std::vector<std::string> some_strings;
  CONFABLE_SCHEMA(AdvancedConfig) {
    return fable::Schema{
        {"some_strings", fable::Schema(&some_strings, "Indicates whether this is cool or not")},
    };
  };
};

// Primary configuration structure
struct ExampleConfig : public fable::Confable {
  bool is_cool = true;
  bool is_supercool = false;
  AdvancedConfig advanced;  // use the nested structure in the primary one

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
        {"is_cool", make_schema(&is_cool, "Indicates whether this is cool or not")},
        {"is_supercool",
         make_schema(&is_supercool, "Indicates whether this is supercool or not").require()},
        {"advanced", make_schema(&advanced, "Demonstrates how cool advance data types are")},
    };
  };
};

int main(int argc, char **argv) {
  // Prepare the argument parser
  CLI::App app("Fable Example");
  std::string filename;
  app.add_option("-f,--config-file", filename, "Config file path");

  CLI11_PARSE(app, argc, argv);

  // Ensure the config filename is specified
  if (!filename.size()) {
    std::cout << "Nothing? Really? Try --help" << std::endl;
    return 1;
  }

  const int indent = 3;
  const int header_width = 80;

  // Load the configuration from the file
  ExampleConfig config;
  try {
    std::cout << fmt::format("Loading config from {}", filename) << std::endl;
    config.from_conf(fable::read_conf(filename));
  } catch (fable::Error e) {
    std::cerr << fmt::format("\n\n{0:=^{1}}\n", " JSON-Validation ", header_width) << std::endl;
    std::cerr << fmt::format("Error: {}", e.what()) << std::endl;
    return 1;
  }

  // Print some arbitrary values from the configuration
  std::cout << fmt::format("This is {}cool!", config.is_cool ? "" : "not ") << std::endl;
  std::cout << fmt::format("This is {}supercool!", config.is_supercool ? "" : "not ") << std::endl;
  std::cout << "Advanced config:" << std::endl;
  for (auto x : config.advanced.some_strings) {
    std::cout << fmt::format("   {}", x) << std::endl;
  }

  // Print the JSON-Schema
  std::cout << fmt::format("\n\n{0:=^{1}}\n", " JSON-Schema ", header_width) << std::endl;
  std::cout << config.schema().json_schema().dump(indent) << std::endl;

  // Print the configuration as JSON
  std::cout << fmt::format("\n\n{0:=^{1}}\n", " JSON-Dump ", header_width) << std::endl;
  std::cout << config.to_json().dump(indent) << std::endl;
}
