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
 * \file cloe/utility/output_serializer_json.cpp
 * \see  cloe/utility/output_serializer_json.hpp
 */

#include <cloe/utility/output_serializer_json.hpp>

#include <string>  // for string

#include <cloe/core.hpp>  // for Logger

namespace cloe {
namespace utility {

const std::string JsonFileSerializer::default_filename = "/tmp/cloe_data";

std::unique_ptr<JsonFileSerializer> make_json_file_serializer(FileTypeEnum type, Logger log) {
  std::unique_ptr<JsonFileSerializer> result;
  switch (type) {
    case JSON_GZIP:
      result = std::unique_ptr<JsonFileSerializer>(std::make_unique<GZipJsonSerializer>(log));
      break;
    case JSON_ZIP:
      result = std::unique_ptr<JsonFileSerializer>(std::make_unique<ZlibJsonSerializer>(log));
      break;
    case JSON:
      result = std::unique_ptr<JsonFileSerializer>(std::make_unique<JsonSerializer>(log));
      break;
    default:
      result = make_json_file_serializer(FileTypeEnum::JSON, log);
      break;
  }
  return result;
}

}  // namespace utility
}  // namespace cloe
