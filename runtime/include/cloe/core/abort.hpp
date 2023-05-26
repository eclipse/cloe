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
 * \file cloe/core/abort.hpp
 * \see  cloe/core.hpp
 */

#pragma once

#include <atomic>     // for atomic_bool
#include <stdexcept>  // for exception

namespace cloe {

/**
 * AbortFlag can be included in a plugin to allow asynchronous aborts to be
 * signalled.
 *
 * The abort() method of a plugin can be called asynchronously and should
 * simply store true in the AbortFlag:
 *
 *     abort_flag.store(true);
 *
 */
using AbortFlag = std::atomic_bool;

/**
 * AsyncAbort should be thrown when an asynchronous abort has been signaled.
 */
class AsyncAbort : public std::exception {
 public:
  using exception::exception;
};

/**
 * Create an abort checkpoint.
 *
 * In places where the plugin code might spin for longer periods, the
 * abort_checkpoint() function can be utilized to provide a place to escape.
 * This will throw the AsyncAbort exception, which should be caught by the
 * simulation framework.
 */
inline void abort_checkpoint(AbortFlag& sig) {
  if (sig.load()) {
    throw AsyncAbort();
  }
}

}  // namespace cloe
