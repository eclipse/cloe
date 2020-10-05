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
 * \file cloe/utility/uid_tracker.hpp
 *
 * This file contains the definition of a unique ID assigner class, which is
 * currently used by some cloe_veh_* projects, but is design generic enough to
 * include in cloe_core.
 *
 * This file is primarily meant for internal use, do not rely on a persistent
 * existence!
 */

#pragma once
#ifndef CLOE_UTILITY_UID_TRACKER_HPP_
#define CLOE_UTILITY_UID_TRACKER_HPP_

#include <cassert>     // for assert
#include <functional>  // for std::function
#include <map>         // for std::map
#include <memory>      // for std::shared_ptr
#include <set>         // for std::set
#include <vector>      // for std::vector

namespace cloe {
namespace utility {

/**
 * UniqueIDTracker maintains a mapping from objects to unique IDs, reusing
 * objects as far as possible.
 *
 * It fulfills the following requirements:
 *
 *  1. InIDs are given the same OutIDs within their time to live.
 *  2. If there are no free IDs, the OutID with the greatest age is removed and
 *     replaced.
 *  3. If there is no free OutID with an age > ttl (time-to-live), then an
 *     exception is thrown.
 *  4. The age for all IDs is increased by a call to Next();
 *
 * Currently, the implementation does not remove a mapped ID unless it has not
 * been seen for ttl time. Therefore, if ttl is 1, then it will be reserved for
 * one cycle it is not seen. A ttl of smaller than one makes no sense, and is
 * considered a programmer error.
 */
class UniqueIDTracker {
  // Tracklet is used internally
  struct Tracklet {
    int in_id;
    int out_id;
    int age;
  };

  int ttl_;  // time to live before reusing
  std::map<int, std::shared_ptr<Tracklet>> assigned_;
  std::set<int> free_;
  std::vector<std::function<void(int in_id, int out_id)>> observers_;

 public:
  /**
   * Create a UniqueIDTracker with the minimum and maximum output ID
   * and the maximum TTL (time-to-live) cycles for each ID after not being
   * seen.
   */
  UniqueIDTracker(int min, int max, int ttl = 1) : ttl_(ttl) {
    assert(min < max);
    assert(ttl_ >= 1);  // TODO(unknown): perhaps 0 is ok too

    for (int i = min; i <= max; i++) {
      free_.insert(i);
    }
  }

  virtual ~UniqueIDTracker() = default;

  /**
   * Return a unique persistent ID across multiple cycles (steps).
   * The input ID is a unique number representable by int (can be random for
   * all the package cares) and is mapped to an int that is within the range
   * specified on creation of the class (see min and max parameters).
   *
   * It is not an error to assign the same input ID twice, the same output ID
   * will be returned.
   *
   * \param id a unique int for an object
   * \return the unique ID mapped between min and max of this class.
   * \throw std::range_error when no more free output IDs available
   */
  int assign(int id);

  /**
   * Advance to the next cycle of the assignments.
   * This removes all mapped IDs if they have exceeded their TTL.
   */
  void next_cycle();

  /**
   * Add an observer that is called with the in and output ID
   * when that ID is removed from the map (i.e., when it expires).
   */
  void add_delete_observer(void f(int in_id, int out_id)) { observers_.emplace_back(f); }
};

}  // namespace utility
}  // namespace cloe

#endif  // CLOE_UTILITY_UID_TRACKER_HPP_
