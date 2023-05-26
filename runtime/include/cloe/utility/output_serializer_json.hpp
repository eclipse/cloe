/*
 * Copyright 2021 Robert Bosch GmbH
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
 * \file output_serializer_json.hpp
 */

#pragma once

#include <string>  // for string

#include <cloe/core.hpp>                       // for Json
#include <cloe/utility/output_serializer.hpp>  // for Outputstream, JSONSerializer, ...

namespace cloe {
namespace utility {

enum class JsonFileType {
  JSON_GZIP,
  JSON_ZIP,
  JSON,
};

// clang-format off
ENUM_SERIALIZATION(JsonFileType, ({
    {cloe::utility::JsonFileType::JSON_GZIP    , "json.gz"     },
    {cloe::utility::JsonFileType::JSON_ZIP     , "json.zip"    },
    {cloe::utility::JsonFileType::JSON         , "json"        },
}))
// clang-format on

class AbstractJsonSerializerBase {
 protected:
  static const std::string json_array_open;
  static const std::string json_array_close;
};

template <typename... TSerializerArgs>
class AbstractJsonSerializer : public Serializer<TSerializerArgs...>,
                               public AbstractJsonSerializerBase {
 public:
  using base = Serializer<TSerializerArgs...>;
  using Serializer<TSerializerArgs...>::Serializer;
  virtual ~AbstractJsonSerializer() = default;
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename + ".json";
  }
  void start_array() override { base::write(json_array_open); }
  void end_array() override { base::write(json_array_close); }
};

class SimpleJsonSerializer : public AbstractJsonSerializer<const Json&, bool> {
 public:
  using json_base = AbstractJsonSerializer<const Json&, bool>;
  using json_base::json_base;
  void serialize(const Json& j, bool write_delim) override {
    if (write_delim) {
      write(",\n");  // serialize delimiting comma, if already one dataset was serialized
    }
    auto txt = j.dump(2);  // serialize with level 2 indent
    write(txt);
  }
};

// JsonFileSerializer is
// 1) Interface for the consumer class
// 2) the anchor point for exactly one instance of the default_filename
class JsonFileSerializer {
 public:
  virtual ~JsonFileSerializer() = default;

  [[nodiscard]]
  virtual bool open_file(const std::string& filename) = 0;

  virtual void serialize(const Json& j) = 0;
  virtual void close_file() = 0;

 protected:
  static const std::string default_filename;
  bool prepend_delimiter{false};
};

// JsonFileSerializerImpl is the implementation of JsonFileSerializer
template <typename TOutputStream>
class JsonFileSerializerImpl
    : public SequentialFileSerializer<SimpleJsonSerializer, TOutputStream, const Json&, bool>,
      public JsonFileSerializer {
  using file_base =
      SequentialFileSerializer<SimpleJsonSerializer, TOutputStream, const Json&, bool>;

 public:
  JsonFileSerializerImpl(Logger logger) : file_base(logger), JsonFileSerializer() {}
  virtual ~JsonFileSerializerImpl() = default;
  using file_base::open_file;

  [[nodiscard]]
  bool open_file(const std::string& filename) override {
    std::string default_name = this->outputstream_.make_default_filename(
        this->serializer_.make_default_filename(default_filename));
    return file_base::open_file(filename, default_name);
  }

  using file_base::serialize;
  void serialize(const Json& j) override {
    file_base::serialize(j, prepend_delimiter);
    prepend_delimiter = true;
  }
  void close_file() override { file_base::close_file(); }

 protected:
  void on_file_opened() override { this->serializer_.start_array(); }
  void on_file_closing() override { this->serializer_.end_array(); }
};

using JsonSerializer = JsonFileSerializerImpl<FileOutputStream>;
using ZlibJsonSerializer = JsonFileSerializerImpl<ZlibOutputStream>;
using GZipJsonSerializer = JsonFileSerializerImpl<GzipOutputStream>;

std::unique_ptr<JsonFileSerializer> make_json_file_serializer(JsonFileType type, Logger log);

}  // namespace utility
}  // namespace cloe
