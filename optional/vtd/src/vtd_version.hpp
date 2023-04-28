/*
 * Copyright 2022 Robert Bosch GmbH
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
 * \file vtd_version.hpp
 *
 * This file just provides version information for the VTD version we
 * are working with.
 *
 * We expect the vtd-api library to define a constant VTD_API_VERSION,
 * which takes the following format:
 *
 *   (EPOCH << 24) | (MAJOR_VERSION << 16) | (MINOR_VERSION << 8) | PATCH_VERSION
 *
 * Each version consists of at most 8 bits, so 256 potential values, including 0.
 * The epoch starts with 0, and is bumped after each version naming scheme.
 *
 * The following macros are defined from VTD_API_VERSION:
 *
 * - VTD_API_VERSION_EPOCH
 * - VTD_API_VERSION_MAJOR
 * - VTD_API_VERSION_MINOR
 * - VTD_API_VERSION_PATCH
 *
 * When years are involved, such as in releases versioned YYYY.MM, use the last
 * two years, since the full year will not fit in the 8 bits.
 *
 * Here are some example versions that could be used with this plugin:
 *
 * - VTD 2.1.0    -> Epoch = 0
 * - VTD 2.2.0
 * - VTD 2019.1   -> Epoch = 1
 * - VTD 2022.3
 *
 * So in order to target VTD 2.X, we could use `VTD_API_VERSION_EPOCH==0` or
 * `VTD_API_VERSION_MAJOR==2`.
 */

#pragma once

#ifndef VTD_API_VERSION
#error "require VTD_API_VERSION to be defined"
#endif

#define VTD_API_VERSION_EPOCH ((VTD_API_VERSION >> 24) && 0xff)
#define VTD_API_VERSION_MAJOR ((VTD_API_VERSION >> 16) && 0xff)
#define VTD_API_VERSION_MINOR ((VTD_API_VERSION >> 8) && 0xff)
#define VTD_API_VERSION_PATCH ((VTD_API_VERSION >> 0) && 0xff)
