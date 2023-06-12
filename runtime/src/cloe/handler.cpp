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
 * \file cloe/handler.cpp
 * \see  cloe/handler.hpp
 */

#include <cloe/handler.hpp>

#include <stdexcept>  // for runtime_error
#include <string>     // for string

#include <cloe/core.hpp>  // for Json, Schema, Confable

namespace cloe {

/**
 * See
 * https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Complete_list_of_MIME_types
 */
const char* as_cstr(const ContentType& t) {
  switch (t) {
    case ContentType::JSON:
      return "application/json";
    case ContentType::HTML:
      return "text/html";
    case ContentType::CSS:
      return "text/css";
    case ContentType::JAVASCRIPT:
      return "application/javascript";
    case ContentType::CSV:
      return "text/csv";
    case ContentType::TEXT:
      return "text/plain";
    case ContentType::SVG:
      return "image/svg+xml";
    case ContentType::PNG:
      return "image/png";
    default:
      return "application/octet-stream";
  }
}

const char* as_cstr(const RequestMethod& m) {
  switch (m) {
    case RequestMethod::GET:
      return "GET";
    case RequestMethod::POST:
      return "POST";
    case RequestMethod::PUT:
      return "PUT";
    case RequestMethod::DELETE:
      return "DELETE";
    default:
      throw std::runtime_error("as_cstr(const RequestMethod&): programming error");
  }
}

void from_string(const std::string& s, RequestMethod& m) {
  if (s == "GET") {
    m = RequestMethod::GET;
  } else if (s == "POST") {
    m = RequestMethod::POST;
  } else if (s == "PUT") {
    m = RequestMethod::PUT;
  } else if (s == "DELETE") {
    m = RequestMethod::DELETE;
  } else {
    throw std::runtime_error("from_string(const std::string&, RequestMethod&): input error");
  }
}

namespace handler {

void FromConf::operator()(const cloe::Request& q, cloe::Response& r) {
  switch (q.method()) {
    case RequestMethod::POST:
      try {
        ptr_->from_conf(Conf{q.as_json()});
      } catch (ConfError& e) {
        r.bad_request(e);
      }
      break;

    case RequestMethod::GET:
      // Try to convert URL query map to a JSON and pass that in,
      // otherwise fallthrough to the default.
      if (convert_ && !q.query_map().empty()) {
        Json j;
        for (auto const& kv : q.query_map()) {
          j[kv.first] = fable::parse_json(kv.second);
        }
        try {
          ptr_->from_conf(Conf{q.as_json()});
        } catch (ConfError& e) {
          r.bad_request(e);
        }
        break;
      }
      [[fallthrough]];

    default:
      r.not_allowed(RequestMethod::POST,
                    Json{
                        {"error", "expect POST method and JSON body"},
                        {"fields", ptr_->schema().usage()},
                    });
  }
}

}  // namespace handler
}  // namespace cloe
