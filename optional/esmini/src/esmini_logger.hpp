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
 * \file esmini_logger.hpp
 *
 * This header contains the namespace logger for the ESMini binding.
 */

#pragma once

#include <cloe/core.hpp>  // for Logger, get

namespace esmini {

inline cloe::Logger esmini_logger() { return cloe::logger::get("esmini"); }

}  // namespace esmini
