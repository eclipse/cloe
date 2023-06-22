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
 * \file fable/schema/boost_optional.hpp
 * \see  fable/schema/optional.cpp
 */

#pragma once

#include <boost/optional/optional_fwd.hpp>  // for optional<>

#include <fable/schema/optional.hpp>         // for is_optional<>
#include <fable/utility/boost_optional.hpp>  // for adl_serializer<>

namespace fable {
namespace schema {

template <typename X>
struct is_optional<boost::optional<X>> : std::true_type {};

}  // namespace schema
}  // namespace fable
