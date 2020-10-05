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
 * \file cloe/trigger/evaluate_event.cpp
 * \see  cloe/trigger/evaluate_event.hpp
 * \see  cloe/utility/evaluate.hpp
 */

#include <cloe/trigger/evaluate_event.hpp>

#include <string>   // for string
#include <utility>  // for move

#include <cloe/utility/evaluate.hpp>  // for compile_evaluation

namespace cloe {
namespace events {

bool Evaluate::operator()(const Sync&, double d) {
#ifndef NDEBUG
  if (func_(d)) {
    logger()->debug("The expression '{}{}' evaluated to true.", d, repr_);
    return true;
  }
  return false;
#else
  return func_(d);
#endif
}

void Evaluate::to_json(Json& j) const {
  j = Json{
      {"is", repr_},
  };
}

TriggerSchema EvaluateFactory::schema() const {
  static const char* desc = "comparison between a variable and a constant";
  return TriggerSchema{
      name(),
      description(),
      InlineSchema(desc, "comparison", true),
      Schema{
          {"is", make_prototype<std::string>("operator followed by constant").require()},
      },
  };
}

EventPtr EvaluateFactory::make(const Conf& c) const {
  try {
    auto repr = c.get<std::string>("is");
    auto f = utility::compile_evaluation(repr);
    return std::make_unique<Evaluate>(name(), repr, std::move(f));
  } catch (std::exception& e) {
    throw TriggerInvalid(c, e.what());
  }
}

EventPtr EvaluateFactory::make(const std::string& s) const {
  return make(Conf{Json{
      {"is", s},
  }});
}

}  // namespace events
}  // namespace cloe
