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
 * \file oak/request_stub.hpp
 */

#pragma once

#include <map>        // for map<>
#include <stdexcept>  // for logic_error
#include <string>     // for string

#include <cloe/handler.hpp>  // for Request, RequestMethod, ContentType

namespace oak {

class RequestStub : public cloe::Request {
 public:
  cloe::RequestMethod method() const override { throw ERROR; }
  cloe::ContentType type() const override { throw ERROR; }
  const std::string& body() const override { throw ERROR; }
  const std::string& uri() const override { throw ERROR; }
  const std::string& endpoint() const override { throw ERROR; }
  const std::map<std::string, std::string>& query_map() const override { throw ERROR; }

 private:
  const std::logic_error ERROR = std::logic_error("using the request in any way is erroneous");
};

}  // namespace oak
