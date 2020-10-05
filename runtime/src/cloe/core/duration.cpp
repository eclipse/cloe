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
 * \file cloe/core/duration.cpp
 * \see  cloe/core/duration.hpp
 */

#include <cloe/core/duration.hpp>

#include <chrono>  // for duration_cast
#include <string>  // for operator+, string, to_string

namespace cloe {
namespace {

std::string to_string_hr(double d) {
  auto n = std::to_string(d);
  n.erase(n.find_last_not_of('0') + 1, std::string::npos);
  if (n[n.size() - 1] == '.') {
    n.pop_back();
  }
  return n;
}

}  // anonymous namespace

std::string to_string(const Duration& ns) {
  auto count = ns.count();
  if (count > 1e9) {
    return to_string_hr(std::chrono::duration_cast<cloe::Seconds>(ns).count()) + "s";
  } else if (count > 1e6) {
    return to_string_hr(std::chrono::duration_cast<cloe::Milliseconds>(ns).count()) + "ms";
  } else if (count > 1e3) {
    return to_string_hr(std::chrono::duration_cast<cloe::Microseconds>(ns).count()) + "us";
  } else {
    return to_string_hr(count) + "ns";
  }
}

nlohmann::json to_convenient_json(const Duration& ns) {
  return nlohmann::json{
      {"str", ns},
      {"ms", std::chrono::duration_cast<std::chrono::duration<uint64_t, std::milli>>(ns).count()},
  };
}

}  // namespace cloe
