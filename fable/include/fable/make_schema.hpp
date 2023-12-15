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
 * \file fable/make_schema.hpp
 *
 * This file provides a facade for the make_schema function which implements
 * a short-cut for the compiler which reduces the excessive resource
 * consumption of the actual implementation.
 */

#pragma once

#include <string> // for string

#include <fable/schema/array.hpp>      // for Array<>
#include <fable/schema/boolean.hpp>    // for Boolean
#include <fable/schema/confable.hpp>   // for FromConfable
#include <fable/schema/const.hpp>      // for Const<>
#include <fable/schema/duration.hpp>   // for Duration<>
#include <fable/schema/enum.hpp>       // for Enum<>
#include <fable/schema/ignore.hpp>     // for Ignore
#include <fable/schema/interface.hpp>  // for Interface, Box
#include <fable/schema/json.hpp>       // for FromJson<>
#include <fable/schema/map.hpp>        // for Map<>
#include <fable/schema/number.hpp>     // for Number<>
#include <fable/schema/optional.hpp>   // for Optional<>
#include <fable/schema/passthru.hpp>   // for Passthru
#include <fable/schema/path.hpp>       // for Path
#include <fable/schema/string.hpp>     // for String
#include <fable/schema/struct.hpp>     // for Struct
#include <fable/schema/variant.hpp>    // for Variant

// It is important that this include comes after all the other ones,
// so that it has access to ALL the previous definitions.
#include <fable/schema/magic.hpp>  // for make_prototype, ...

namespace fable::schema {
namespace details {

/**
 * Implements the make_schema function for one datatype
 */
template <typename T>
struct make_schema_wrapper1 {
  static auto make_schema(T* ptr, std::string&& desc) {
    return ::fable::schema::make_schema_impl(ptr, std::move(desc));
  }
};

/**
 */
template <typename T, typename P>
struct make_schema_wrapper2 {
  static auto make_schema(T* ptr, P proto, std::string&& desc) {
    return ::fable::schema::make_schema_impl(ptr, std::move(proto), std::move(desc));
  }
};

}  // namespace details

/**
 * Returns the schema for a given datum and its description
 *
 * \param ptr Pointer to the datum
 * \param desc Textual description of the datum
 * \return Schema for the datum
 */
template <typename T>
auto make_schema(T* ptr, std::string&& desc) {
  return details::make_schema_wrapper1<T>::make_schema(ptr, std::move(desc));
}

/**
 * Returns the schema for a given datum and its description
 *
 * \param ptr Pointer to the datum
 * \param proto Prototype of the data-value
 * \param desc Textual description of the datum
 * \return Schema for the datum
 * \note Those types which have a prototype, namely string & path
 *       do not matter for the compile-time reduction
 */
template <typename T, typename P>
auto make_schema(T* ptr, P proto, std::string&& desc) {
  return details::make_schema_wrapper2<T, P>::make_schema(ptr, std::move(proto), std::move(desc));
}

}  // namespace fable::schema
