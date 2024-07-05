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
 * \file oak/route_muxer_test.cpp
 * \see  oak/route_muxer.hpp
 */

#include <gtest/gtest.h>  // for TEST, EXPECT_TRUE, ...

#include <map>        // for map<>
#include <string>     // for string
#include <utility>    // for tie
#include <vector>     // for vector<>
using namespace std;  // NOLINT(build/namespaces)

#include <oak/route_muxer.hpp>  // for Muxer<>
using oak::Muxer;
using oak::Parameters;

TEST(oak_muxer, normalize) {
  vector<pair<string, string>> tests{
      {"", ""},
      {"/", "/"},
      {"/.", "/"},
      {"abc", ""},
      {"/abc", "/abc"},
      {"/abc/", "/abc"},
      {"/abc?", "/abc"},
      {"/abc/?opt=/next", "/abc"},
      {"C:", ""},
      {"//", "/"},
      {"/..", "/"},
      {"/abc//.", "/abc"},
      {"/index.html", "/index.html"},
      {"/favicon.png", "/favicon.png"},
  };

  for (auto p : tests) {
    auto result = Muxer<bool>::normalize(p.first);
    EXPECT_TRUE(result == p.second)
        << "Normalize('" << p.first << "') = " << result << ", expected '" << p.second << "'";
  }
}

TEST(oak_muxer, resolve) {
  Muxer<bool> mux;
  mux.set_default(false);
  vector<string> paths{"/", "/abc"};
  for (auto p : paths) {
    mux.add(p, true);
  }

  vector<string> ok{"/", "/abc", "/abc?yes"};
  for (auto p : ok) {
    EXPECT_TRUE(mux.get(p).first);
  }
  vector<string> nok{"/none", "/abc/next"};
  for (auto p : nok) {
    EXPECT_FALSE(mux.get(p).first);
  }
}

TEST(oak_muxer, resolve_with_parameters) {
  Muxer<bool> mux;
  mux.set_backtrack(true);
  mux.set_default(false);

  vector<string> paths{"/vehicles", "/vehicles/{name}", "/vehicles/{name}/components/{component}"};
  for (auto p : paths) {
    mux.add(p, true);
  }

  vector<string> ok{"/vehicles", "/vehicles/a", "/vehicles/rori?"};
  for (auto p : ok) {
    EXPECT_TRUE(mux.get(p).first);
  }

  Parameters p;
  bool result;
  tie(result, p) = mux.get("/vehicles/a");
  EXPECT_TRUE(result);
  // TODO(ben): continue testing here
}

TEST(oak_muxer, resolve_with_backtrack) {
  Muxer<bool> mux;
  mux.set_backtrack(true);
  mux.set_default(false);
  vector<string> paths{"/index.html", "/favicon.png", "/cloe/state"};
  for (auto p : paths) {
    mux.add(p, true);
  }

  vector<string> ok{"/index.html", "/favicon.png", "/cloe/state?", "/cloe/state/s"};
  for (auto p : ok) {
    EXPECT_TRUE(mux.get(p).first);
  }
  vector<string> nok{"/", "/cloe", "/abc?yes", "/abc/next", "/none"};
  for (auto p : nok) {
    EXPECT_FALSE(mux.get(p).first) << p << " resolves to " << mux.resolve(p);
  }
}
