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
 * \file gndtruth_extractor.hpp
 */

#include <cloe/core.hpp>  // for Confable, Schema

namespace cloe {
enum class OutputTypeEnum {
  JSON_BZIP2,
  JSON_GZIP,
  JSON_ZIP,
  JSON,
  MSGPACK_BZIP2,
  MSGPACK_GZIP,
  MSGPACK_ZIP,
  MSGPACK,
};

// clang-format off
ENUM_SERIALIZATION(OutputTypeEnum, ({
    {cloe::OutputTypeEnum::JSON_BZIP2   , "json.bz2"    },
    {cloe::OutputTypeEnum::JSON_GZIP    , "json.gz"     },
    {cloe::OutputTypeEnum::JSON_ZIP     , "json.zip"    },
    {cloe::OutputTypeEnum::JSON         , "json"        },
    {cloe::OutputTypeEnum::MSGPACK_BZIP2, "msgpack.bz2" },
    {cloe::OutputTypeEnum::MSGPACK_GZIP , "msgpack.gz"  },
    {cloe::OutputTypeEnum::MSGPACK_ZIP  , "msgpack.zip" },
    {cloe::OutputTypeEnum::MSGPACK      , "msgpack"     },
}))
// clang-format on

namespace controller {
struct GndTruthExtractorConfiguration : public Confable {
  std::string output_file;
  cloe::OutputTypeEnum output_type{cloe::OutputTypeEnum::JSON_GZIP};
  std::vector<std::string> components;

  CONFABLE_SCHEMA(GndTruthExtractorConfiguration) {
    return Schema{
        {"components", Schema(&components, "array of components to be extracted")},
        {"output_file", Schema(&output_file, "file path to write groundtruth output to")},
        {"output_type", Schema(&output_type, "type of output file to write")},
    };
  }
};

}  // namespace controller
}  // namespace cloe
