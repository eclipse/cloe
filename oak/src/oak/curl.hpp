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
 * \file oak/curl.hpp
 */

#pragma once

#include <string>
#include <vector>

namespace oak {

struct Curl {
  std::string method;
  std::string address;
  int port;
  std::string endpoint;
  std::string data;
  std::vector<std::string> headers;

 public:
  static Curl get(const std::string& address, int port, const std::string& endpoint) {
    Curl c;
    c.method = "GET";
    c.address = address;
    c.port = port;
    c.endpoint = endpoint;
    return std::move(c);
  }

  static Curl post(const std::string& address, int port, const std::string& endpoint,
                   const std::string& data, const std::string& mime_type) {
    Curl c;
    c.method = "POST";
    c.address = address;
    c.port = port;
    c.endpoint = endpoint;
    c.data = data;
    c.headers.push_back(std::string("Content-Type: ") + mime_type);
    return c;
  }

  std::string to_string() const {
    std::string buf;
    buf += "curl -q";
    buf += " -X " + method;
    for (const auto& h : headers) {
      buf += " -H '" + h + "'";
    }
    if (data != "") {
      buf += " -d '" + data + "'";
    }
    buf += " http://" + address + ":" + std::to_string(port);
    if (endpoint != "" && endpoint[0] != '/') {
      buf += "/";
    }
    buf += endpoint;
    return buf;
  }
};

}  // namespace oak
