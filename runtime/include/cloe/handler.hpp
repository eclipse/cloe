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
 * \file cloe/handler.hpp
 * \see  cloe/handler.cpp
 *
 * This file contains definitions required for defining HTTP handlers for
 * endpoints.
 *
 * For example, there are simple methods that can be used if all
 * a class wants to do is create an endpoint that returns a JSON object.
 * This file does not specify who acquires handlers from classes, only
 * what those handlers look like.
 *
 * # Dependencies
 *
 * This file should not have any dependencies to the rest of Cloe apart from
 * what is used as a JSON class. As such, it should be understandable by itself.
 */

#pragma once

#include <functional>  // for function
#include <map>         // for map
#include <string>      // for string
#include <utility>     // for pair

#include <fable/confable.hpp>  // for Confable
#include <fable/json.hpp>      // for Json

namespace cloe {

/**
 * An enumeration of the most common request methods.
 *
 * If a request comes that is unrecognized, UNKNOWN is returned.
 * Currently, the primary methods that we recognize are GET and POST.
 */
enum class RequestMethod {
  GET = 1,
  POST = 2,
  PUT = 4,
  DELETE = 8,
};

const char* as_cstr(const RequestMethod& m);
void from_string(const std::string& s, RequestMethod& m);

/**
 * An enumeration of the most common content types.
 *
 * When returning a response to the web browser, it inspects the content type
 * to determine what to do with the response. It is therefore important that we
 * set the correct content type on responses.
 */
enum class ContentType {
  NOT_APPLICABLE,
  UNKNOWN,
  JSON,
  HTML,
  CSS,
  CSV,
  JAVASCRIPT,
  TEXT,
  SVG,
  PNG,
};

const char* as_cstr(const ContentType& t);

/**
 * A Request represents an HTTP request sent by an external client.
 *
 * This may contain data, for example when the request is a POST request.
 *
 * - The interface here is fully const, as it does not make sense to modify
 *   a request.
 */
class Request {
 public:
  virtual ~Request() {}

  /**
   * Returns what method was used on this request, for example GET or POST.
   */
  virtual RequestMethod method() const = 0;

  /**
   * Returns the type, if any, of the content.
   *
   * Note that the client may not send the right content type for the content.
   * This function checks the Content-Type header, and if that is not set,
   * it may optionally check the body itself.
   *
   * - If there is no content, NOT_APPLICABLE is returned.
   * - If there is content, but we don't know what kind of data, then UNKNOWN
   *   is returned.
   */
  virtual ContentType type() const = 0;

  /**
   * Returns the body of the request as a reference to a string.
   *
   * - If the request does not contain a body, the return value is an empty
   *   string.
   */
  virtual const std::string& body() const = 0;

  /**
   * Returns the path, including query string.
   */
  virtual const std::string& uri() const = 0;

  /**
   * Returns just the endpoint part of the URI.
   *
   * For example:
   *
   *  https://localhost:8080/cloe/simulation?type=json => /cloe/simulation
   */
  virtual const std::string& endpoint() const = 0;

  /**
   * Returns a key-value map of the query parameters.
   */
  virtual const std::map<std::string, std::string>& query_map() const = 0;

  /**
   * Helper method that returns whether the header specifies that there is JSON
   * data.
   */
  virtual bool has_json() const { return this->type() == ContentType::JSON; }

  /**
   * Helper method that tries to convert the body to a JSON object.
   */
  virtual fable::Json as_json() const { return fable::parse_json(body()); }
};

/**
 * An enumeration of the most common status codes.
 *
 * For more information, see the RFC:
 * https://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
 */
enum class StatusCode {
  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  NO_CONTENT = 204,
  RESET_CONTENT = 205,
  PARTIAL_CONTENT = 206,
  MULTIPLE_CHOICES = 300,
  MOVED_PERMANENTLY = 301,
  FOUND = 302,
  SEE_OTHER = 303,
  NOT_MODIFIED = 304,
  USE_PROXY = 305,
  TEMPORARY_REDIRECT = 307,
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  NOT_ALLOWED = 405,
  NOT_ACCEPTABLE = 406,
  REQUEST_TIMEOUT = 408,
  CONFLICT = 409,
  GONE = 410,
  SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  SERVICE_UNAVAILABLE = 503,
};

/**
 * A Response is passed to a Handler, which can set its fields.
 *
 * - If the status code is not set, then 200 or 204 is set depending on whether
 *   any content was written or not.
 * - In contrast to a Request, a Response is not inheritable.
 */
class Response {
 public:
  /**
   * The default response if nothing else is done is the 204 No Content response.
   */
  Response() : status_(StatusCode::NO_CONTENT), type_(ContentType::NOT_APPLICABLE) {}

  /**
   * Returns a map of headers set by the server.
   *
   * # Conformity
   *
   * This is theoretically conform to the HTTP specification. According to
   * [HTTP 1.1 Section 4.2](http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.2):
   *
   * > Multiple message-header fields with the same field-name MAY be present in
   * > a message if and only if the entire field-value for that header field is
   * > defined as a comma-separated list [i.e., #(values)]. It MUST be possible
   * > to combine the multiple header fields into one "field-name: field-value"
   * > pair, without changing the semantics of the message, by appending each
   * > subsequent field-value to the first, each separated by a comma. The order
   * > in which header fields with the same field-name are received is therefore
   * > significant to the interpretation of the combined field value, and thus
   * > a proxy MUST NOT change the order of these field values when a message is
   * > forwarded.
   *
   * However, for the `Set-Cookie` header, there are several non-conforming
   * syntaxes, which make use of the comma. Since our server currently does not
   * set cookies with multiple expiration times, this is a non-issue for us.
   */
  const std::map<std::string, std::string>& headers() const { return headers_; }
  std::map<std::string, std::string>& headers() { return headers_; }

  /**
   * Return whether the header map has the given header set.
   */
  bool has_header(const std::string& key) { return this->headers().count(key) != 0; }

  /**
   * Return the value of the specified header or throw an exception.
   *
   * - Use HasHeader or the Headers map directly if you do not want to handle
   *   an exception.
   */
  const std::string& header(const std::string& key) { return this->headers().at(key); }

  /**
   * This will set the header with the key to value, overwriting any previous
   * value.
   */
  void set_header(const std::string& key, const std::string& value) {
    this->headers()[key] = value;
  }

  StatusCode status() const { return status_; }
  void set_status(StatusCode code) { status_ = code; }

  ContentType type() const { return type_; }
  void set_type(ContentType type) {
    type_ = type;
    this->set_header("Content-Type", as_cstr(type));
  }

  const std::string& body() const { return body_; }

  /**
   * Sets the body of the response.
   */
  void set_body(const std::string& s, ContentType type) {
    if (!s.empty() && status_ == StatusCode::NO_CONTENT) {
      status_ = StatusCode::OK;
    }
    this->set_type(type);
    body_ = s;
  }

  /**
   * Set the body to the JSON and set the content type.
   *
   * - When NDEBUG is set, then the serialization of JSON to string does not
   *   pretty print.
   */
  void set_body(const fable::Json& js) {
#ifdef NDEBUG
    this->set_body(js.dump(), ContentType::JSON);
#else
    this->set_body(js.dump(4), ContentType::JSON);
#endif
  }

  /**
   * Write is an alias for `set_body(Json)`, for historical reasons.
   */
  void write(const fable::Json& js) { this->set_body(js); }

  /**
   * Use bad_request when the method is correct, but the body content is not
   * correct.
   */
  void bad_request(const fable::Json& js) { this->error(StatusCode::BAD_REQUEST, js); }

  /**
   * Use not_found when the resource in question is not available.
   */
  void not_found(const fable::Json& js) { this->error(StatusCode::NOT_FOUND, js); }

  /**
   * Use not_allowed when the method (GET, POST, PUT, DELETE) is not allowed.
   *
   * Specify in `allow` which method is allowed:
   * ```
   * r.not_allowed(RequestMethod::POST, fable::Json{{"error", "try something else"}});
   * ```
   */
  void not_allowed(const RequestMethod& allow, const fable::Json& js) {
    this->set_status(StatusCode::NOT_ALLOWED);
    this->set_header("Allow", as_cstr(allow));
    this->set_body(js);
  }

  /**
   * Use not_implemented when the functionality represented by the endpoint
   * is not implemented yet.
   */
  void not_implemented(const fable::Json& js) { this->error(StatusCode::NOT_IMPLEMENTED, js); }

  /**
   * Use server_error when an internal error occurred, such as a panic.
   */
  void server_error(const fable::Json& js) { this->error(StatusCode::SERVER_ERROR, js); }

  void error(StatusCode code, const fable::Json& js) {
    this->set_body(js);
    this->set_status(code);
  }

 private:
  StatusCode status_;
  ContentType type_;
  std::string body_;
  std::map<std::string, std::string> headers_;
};

/**
 * A Handler receives a Request and a Response and can be registered with
 * a server and an endpoint.
 *
 * # Future
 *
 * In the future, this type may be converted from a simple function to a class.
 */
using Handler = std::function<void(const Request&, Response&)>;

namespace handler {

/**
 * The Redirect handler redirects to the location it is created with.
 *
 * Note: This is not a permanent redirect, it approximately sends the following
 * response:
 *
 *     302 Found
 *     Location /your/location/here
 *
 */
class Redirect {
 public:
  explicit Redirect(const std::string& location) : location_(location) {}
  void operator()(const cloe::Request&, cloe::Response& r) {
    r.set_status(StatusCode::FOUND);
    r.set_header("Location", location_);
  }

 private:
  std::string location_;
};

/**
 * StaticJson serves static content from whatever can be converted into json.
 */
class StaticJson {
 public:
  StaticJson(fable::Json j) : data_(j) {}  // NOLINT
  void operator()(const cloe::Request&, cloe::Response& r) { r.write(data_); }

 private:
  const fable::Json data_;
};

/**
 * The ToJson handler tries to convert its input to JSON.
 *
 * - It requires a pointer as input, as it assumes that the data will change.
 * - The type of the pointer must have the associated `to_json` function
 *   implemented. In other words, it must be directly convertible to a
 *   `fable::Json`.
 */
template <typename T>
class ToJson {
 public:
  explicit ToJson(const T* ptr) : ptr_(ptr) {}
  void operator()(const cloe::Request&, cloe::Response& r) {
    fable::Json j;
    to_json(j, *ptr_);
    r.set_body(j);
  }

 private:
  const T* ptr_;
};

/**
 * The FromConf handler tries to convert its input to a Conf.
 *
 * Requests are handled in the following way:
 *
 * - If the request uses the POST method, the body is parsed as JSON and the
 *   `from_json` function is called for the type.
 * - If the request uses the GET method and supplies a query map, then the
 *   query map is converted to JSON and passed in (this behavior enabled by
 *   default).
 * - Otherwise, return the schema usage of the Confable with an error.
 */
class FromConf {
 public:
  explicit FromConf(fable::Confable* ptr, bool query_map_as_json = true)
      : ptr_(ptr), convert_(query_map_as_json) {}
  void operator()(const cloe::Request& q, cloe::Response& r);

 private:
  fable::Confable* ptr_;
  bool convert_;
};

}  // namespace handler
}  // namespace cloe
