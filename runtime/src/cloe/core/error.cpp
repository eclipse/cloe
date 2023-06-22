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
 * \file cloe/core/error.cpp
 * \see  cloe/core/error.hpp
 */

#include <cloe/core/error.hpp>

#include <string>  // for string

#include <boost/algorithm/string.hpp>  // for replace_all

namespace cloe {

void Error::set_explanation(std::string s) {
  // Writing long explanations is a PITA, so we do some preprocessing on the
  // string s to make it nicer:
  //
  //    If s starts with a newline, then the following string of spaces is
  //    interpreted as the amount to be removed following every newline.
  explanation_ = std::move(s);
  if (explanation_.empty() || explanation_[0] != '\n') {
    return;
  }

  auto pos = explanation_.find_first_not_of("\n ");
  if (pos != std::string::npos) {
    auto txt = explanation_.substr(0, pos);
    boost::replace_all(explanation_, txt, "\n");
  }

  auto last = explanation_.size() - 1;
  if (explanation_[last] == '\n') {
    last--;
  }
  explanation_ = explanation_.substr(1, last);
}

}  // namespace cloe
