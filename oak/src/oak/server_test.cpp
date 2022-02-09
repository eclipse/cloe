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
 * \file oak/server_test.cpp
 * \see  oak/server_test.hpp
 */

#include <gtest/gtest.h>  // for TEST, EXPECT_TRUE, ...

#include <iostream>
#include <map>      // for map<>
#include <string>   // for string
#include <utility>  // for tie
#include <vector>   // for vector<>

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <boost/format.hpp>

#include "cloe/registrar.hpp"

#include "oak/registrar.hpp"  // for Registrar
#include "oak/server.hpp"     // for Server

using namespace std;  // NOLINT(build/namespaces)

/**
 * Executes a (shell) command and return the StdOut
 */
std::string exec(const char* cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}
std::string exec(const std::string& cmd) { return exec(cmd.c_str()); }

std::string address;
std::atomic<unsigned short> port;

class Environment : public ::testing::Environment {
 public:
  ~Environment() override {}
  void SetUp() override {
    address = "127.0.0.1";
    port = 1234;
  }
  void TearDown() override {}
};
testing::Environment* const foo_env = testing::AddGlobalTestEnvironment(new Environment);

std::unique_ptr<oak::Server> create_server() {
  int retry = 0;
  do {
    try {
      auto port_ = port++;
      std::cout << "Trying " << address << ":" << port_ << std::endl;
      auto server = std::make_unique<oak::Server>(address, port_);
      server->listen();
      // if server is listening, return otherwise retry
      if (server->is_listening()) {
        return server;
      } else {
        retry++;
      }
    } catch (...) {
      EXPECT_TRUE(false) << "Unexpected exception";
    }
  } while (retry < 100);
  return nullptr;
}

/**
 * @brief Tries to create a server
 */
TEST(oak_server, listen) { auto server = create_server(); }

/**
 * @brief Tests that stopping a not-listening server results in an exception
 *
 */
TEST(oak_server, fail_not_listening) {
  try {
    oak::Server server;
    server.stop();

    FAIL() << "Expected std::runtime_error";
  } catch (std::runtime_error) {
    SUCCEED();
  } catch (std::exception) {
    FAIL() << "Expected std::runtime_error";
  }
}

/**
 * @brief Tries to GET a non-existing endpoint
 */
TEST(oak_server, get_nohandler) {
  // Create server
  auto server = create_server();
  // Read a non-existing endpoint
  auto address_ = server->address();
  auto port_ = server->port();
  const auto cmd =
      boost::str(boost::format("curl -X GET %1%:%2%/test 2>/dev/null") % address_ % port_);
  auto result = exec(cmd);

  // Compare result
  EXPECT_STREQ(result.c_str(), "{\"endpoints\":[],\"error\":\"cannot find handler\"}");
}

/**
 * @brief Tries to GET an endpoint
 */
TEST(oak_server, get_handler) {
  // Create server
  auto server = create_server();
  // Create registrar
  oak::StaticRegistrar registrar(server.get(), "", nullptr);
  // Register one pseudo-endpoint
  std::vector<std::string> data{
      "1",
      "2",
      "3",
  };
  registrar.register_handler("/simulators", cloe::handler::StaticJson(data));

  // Read the pseudo-endpoint
  auto address_ = server->address();
  auto port_ = server->port();
  const auto cmd =
      boost::str(boost::format("curl -X GET %1%:%2%/simulators 2>/dev/null") % address_ % port_);
  auto result = exec(cmd);

  // Compare result
  EXPECT_STREQ(result.c_str(), "[\"1\",\"2\",\"3\"]");
}

/**
 * @brief Tries to POST to an endpoint which echos the data back
 */
TEST(oak_server, post_handler) {
  // Create server
  auto server = create_server();
  // Create registrar
  oak::StaticRegistrar registrar(server.get(), "", nullptr);
  // Register one pseudo-endpoint
  std::vector<std::string> data{
      "1",
      "2",
      "3",
  };
  registrar.register_handler(
      "/echo", [this](const cloe::Request& request, cloe::Response& response) {
        switch (request.method()) {
          case cloe::RequestMethod::POST: {
            try {
              const auto json = request.as_json();
              response.write(json);
              response.set_status(cloe::StatusCode::OK);
            } catch (const std::exception& ex) {
              response.bad_request(cloe::Json{
                  {"error", ex.what()},
              });
            }
            break;
          }
          default: {
            response.not_allowed(cloe::RequestMethod::POST,
                                 cloe::Json{
                                     {"error", "only GET or POST method allowed"},
                                 });
          }
        }
      });

  // Read the pseudo-endpoint
  auto address_ = server->address();
  auto port_ = server->port();
  const auto cmd =
      boost::str(boost::format("curl -X POST -H \"Content-Type: application/json\" -d '{\"a\": "
                               "\"b\", \"c\": \"d\"}' %1%:%2%/echo 2>/dev/null") %
                 address_ % port_);
  auto result = exec(cmd);

  // Compare result
  EXPECT_STREQ(result.c_str(), "{\"a\":\"b\",\"c\":\"d\"}");
}
