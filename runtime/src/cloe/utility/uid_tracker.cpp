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
 * \file cloe/utility/uid_tracker.cpp
 * \see  cloe/utility/uid_tracker.hpp
 */

#include <cloe/utility/uid_tracker.hpp>

#include <memory>     // for std::shared_ptr
#include <stdexcept>  // for std::range_error
#include <utility>    // for std::pair

#include <cloe/core.hpp>  // for Logger

namespace cloe {
namespace utility {

namespace {
Logger logr() { return logger::get("cloe/uid_tracker"); }
}  // anonymous namespace

int UniqueIDTracker::assign(int id) {
  if (assigned_.count(id) != 0) {
    // Already exists, just return the mapped ID and update the age
    auto t = assigned_[id];
    t->age = 0;  //< reset the age because we added it again
    return t->out_id;
  }

  // Are there any IDs left over?
  if (free_.empty()) {
    logr()->critical("There are no more free output IDs available.");
    for (auto it = assigned_.begin(); it != assigned_.end(); ++it) {
      auto tp = it->second;
      logr()->info("{{ in: {},\tout: {},\tage: {} }}", tp->in_id, tp->out_id, tp->age);
    }
    throw new std::range_error("there are no free output IDs available");
  }

  Tracklet* t = new Tracklet{id, *free_.begin(), 0};
  free_.erase(t->out_id);
  assigned_[id] = std::shared_ptr<Tracklet>(t);
  return t->out_id;
}

void UniqueIDTracker::next_cycle() {
  // We want an ordinary for loop here because we are modifying the map
  for (auto it = assigned_.begin(); it != assigned_.end(); /* no increment */) {
    std::shared_ptr<Tracklet> t = it->second;
    if (t->age >= ttl_) {
      it = assigned_.erase(it);  // since C++11
      free_.insert(t->out_id);
      for (const auto& f : observers_) {
        f(t->in_id, t->out_id);
      }
    } else {
      ++(t->age);
      ++it;
    }
  }
}

}  // namespace utility
}  // namespace cloe
