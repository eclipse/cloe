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
 * \file cloe/core/fable.hpp
 * \see  cloe/core.hpp
 */

#pragma once

#include <fable/fable.hpp>

namespace cloe {

using fable::Conf;
using fable::Confable;
using fable::ConfError;
using fable::Json;
using fable::JsonPointer;
using fable::JsonType;
using fable::Schema;
using fable::SchemaError;

using fable::make_const_schema;
using fable::make_prototype;
using fable::make_schema;
using fable::schema_type;

namespace schema {

using namespace fable::schema;  // NOLINT(build/namespaces)

}  // namespace schema

}  // namespace cloe
