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
 * \file cloe/utility/resource_handler.hpp
 */

#pragma once
#ifndef CLOE_UTILITY_RESOURCE_HANDLER_HPP_
#define CLOE_UTILITY_RESOURCE_HANDLER_HPP_

#include <fstream>
#include <string>  // for string

#include <cloe/handler.hpp>           // for Request, Response
#include <cloe/utility/resource.hpp>  // for ResourceHandler

#ifndef NDEBUG
#include <cloe/core.hpp>  // for Logger
#endif

#define RESOURCE_HANDLER(resource, ct) ::cloe::utility::ResourceHandler(RESOURCE(resource), ct)
#define RESOURCE_LOADER(resource) ::cloe::utility::ResourceLoader(RESOURCE(resource))

namespace cloe {
namespace utility {

/**
 * Makes the resource available as a string normally loading it from the
 * static incbin resource or in debug mode reading it from disk at each access.
 * The resource can made available as Json object.
 */
class ResourceLoader {
 public:
  explicit ResourceLoader(Resource r) : res_(r) {}

  std::string to_string() const {
#ifndef NDEBUG
    // Check if the file given in path exists, and if so, load it.
    std::ifstream ifs{res_.filepath()};
    std::string data;
    if (ifs.is_open()) {
      data = static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str();
    } else {
      logger::get("cloe")->warn("File does not exist: {}", res_.filepath());
      data = res_.to_string();
    }
    return data;
#else
    return res_.to_string();
#endif
  }

  friend void to_json(cloe::Json& j, const ResourceLoader& c);

 private:
  Resource res_;
};

class ResourceHandler {
 public:
  ResourceHandler(Resource r, ContentType t) : resl_(r), type_(t) {}
  void operator()(const Request&, Response& r) { r.set_body(resl_.to_string(), type_); }

 private:
  ResourceLoader resl_;
  ContentType type_;
};

/**
 * Serializes a loaded resource as Json object.
 * In case the resource does not have Json content, its content is serialized
 * as a key value pair {'filepath': 'content'}.
 */
void to_json(cloe::Json& j, const ResourceLoader& c) {
  try {
    j = fable::parse_json(c.to_string());
  } catch (cloe::Json::type_error& e) {
    j = {c.res_.filepath(), c.to_string()};
  }
}

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_RESOURCE_HANDLER_HPP_
