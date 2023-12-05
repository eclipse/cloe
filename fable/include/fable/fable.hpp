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
 * \file fable/fable.hpp
 *
 * This file includes all fable headers so the users don't need to know exactly
 * which headers they need. This is acceptable since most users will end up
 * needing all the headers anyway.
 */

#pragma once

#include <fable/conf.hpp>
#include <fable/confable.hpp>
#include <fable/enum.hpp>
#include <fable/error.hpp>
#include <fable/json.hpp>
#include <fable/json/with_std.hpp>
#include <fable/schema.hpp>
