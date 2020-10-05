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
 * \file cloe/utility/resource.hpp
 */

#pragma once
#ifndef CLOE_UTILITY_RESOURCE_HPP_
#define CLOE_UTILITY_RESOURCE_HPP_

#include <cstddef>
#include <string>

#ifdef INCBIN_HDR
#error "The incbin.h header should only be included by this file."
#endif
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX _blob_
#include <incbin.h>

// These definitions are necessary because order of ## evaluation is undefined.
#define _BLOB_DATA(xName) _CONCAT(_BLOB_X(xName), _data)
#define _BLOB_PATH(xName) _CONCAT(_BLOB_X(xName), _path)
#define _BLOB_END(xName) _CONCAT(_BLOB_X(xName), _end)
#define _BLOB_SIZE(xName) _CONCAT(_BLOB_X(xName), _size)
#define _BLOB_X(xName) _CONCAT(_blob_, xName)
#define _CONCAT(a, b) _CONCAT2(a, b)
#define _CONCAT2(a, b) a##b

/**
 * This macro must be used instead of INCBIN.
 */
#define INCLUDE_RESOURCE(xName, xFilePath)   \
  const char* _BLOB_PATH(xName) = xFilePath; \
  INCBIN(xName, xFilePath)

#define RESOURCE(xName) \
  ::Resource(_BLOB_DATA(xName), _BLOB_END(xName), _BLOB_SIZE(xName), _BLOB_PATH(xName))

class Resource {
 public:
  Resource(const unsigned char* data, const unsigned char* end, const unsigned int len,
           const char* filepath)
      : data_(reinterpret_cast<const char*>(data))
      , end_(reinterpret_cast<const char*>(end))
      , size_(len)
      , filepath_(filepath) {}

  std::string to_string() const { return std::string(data_, size_); }

  size_t size() const { return size_; }
  const char* begin() const { return data_; }
  const char* end() const { return data_ + size_; }
  const char* filepath() const { return filepath_; }

 private:
  const char* data_;
  const char* end_;
  const size_t size_;
  const char* filepath_;
};

#endif  // CLOE_UTILITY_RESOURCE_HPP_
