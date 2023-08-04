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

#pragma once

#include <iostream>

#include <cloe/core/error.hpp>  // for Error
#include <fable/error.hpp>      // for ConfError, SchemaError
#include <fable/utility.hpp>    // for indent_string, pretty_print

namespace cloe {

inline std::string format_error(const std::exception& exception) {
  std::stringstream buf;
  if (const auto* err = dynamic_cast<const fable::SchemaError*>(&exception); err) {
    fable::pretty_print(*err, buf);
  } else if (const auto* err = dynamic_cast<const fable::ConfError*>(&exception); err) {
    fable::pretty_print(*err, buf);
  } else if (const auto* err = dynamic_cast<const cloe::Error*>(&exception); err) {
    buf << err->what() << "\n";
    if (err->has_explanation()) {
      buf << "    Note:\n";
      buf << fable::indent_string(err->explanation(), "    ");
    }
  } else {
    buf << exception.what();
  }
  return buf.str();
}

template <typename Func>
auto conclude_error(std::ostream& out, Func f) -> decltype(f()) {
  try {
    return f();
  } catch (cloe::ConcludedError&) {
    // Has already been logged.
    throw;
  } catch (std::exception& err) {
    out << "Error: " << format_error(err) << std::endl;
    throw cloe::ConcludedError(err);
  }
}

}  // namespace cloe
