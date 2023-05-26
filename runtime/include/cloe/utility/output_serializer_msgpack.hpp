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
#include <cloe/utility/output_serializer.hpp>  // for Serializer

namespace cloe {
namespace utility {

enum class MsgPackFileType {
  MSGPACK_GZIP,
  MSGPACK_ZIP,
  MSGPACK,
};

// clang-format off
ENUM_SERIALIZATION(MsgPackFileType, ({
    {cloe::utility::MsgPackFileType::MSGPACK_GZIP , "msgpack.gz"  },
    {cloe::utility::MsgPackFileType::MSGPACK_ZIP  , "msgpack.zip" },
    {cloe::utility::MsgPackFileType::MSGPACK      , "msgpack"     },
}))
// clang-format on

template <typename T, typename... TSerializerArgs>
class AbstractMsgPackSerializer : public Serializer<TSerializerArgs...> {
 public:
  using base = Serializer<TSerializerArgs...>;
  using Serializer<TSerializerArgs...>::Serializer;
  virtual ~AbstractMsgPackSerializer() = default;
  std::string make_default_filename(const std::string& default_filename) override {
    return default_filename + ".msg";
  }
  void start_array() override {}
  void end_array() override { base::write(Json::to_msgpack(Json(data_))); }

 protected:
  std::vector<T> data_;
};

}  // namespace utility
}  // namespace cloe
