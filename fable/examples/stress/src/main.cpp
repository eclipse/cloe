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
 * \file fable/examples/stress/src/main.cpp
 *
 * In this example application, we will stress-test the compilation.
 * We will be re-using types from the contacts example.
 */

#include <iostream>  // for std::{cout, cerr}
#include <string>    // for std::string<>
#include <vector>    // for std::vector<>

#include <fmt/format.h>        // for fmt::format
#include <CLI/CLI.hpp>         // for CLI::App
#include <boost/optional.hpp>  // for boost::optional<>

#include <fable/confable.hpp>  // for fable::{Confable, CONFABLE_SCHEMA}
#include <fable/schema.hpp>    // for fable::{Schema, String}
#include <fable/utility.hpp>   // for fable::{read_conf}

#include "large_struct.hxx"

int main(int argc, char** argv) {
  // Parse command line arguments:
  CLI::App app("Fable Stress Test Example");
  std::string filename;
  CLI11_PARSE(app, argc, argv);

  Large large;
  std::cout << large.schema().to_json().dump(2) << std::endl;
}
