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
 * \file scp_action.cpp
 * \see  scp_action.hpp
 */

#include "scp_action.hpp"

#include <cloe/trigger.hpp> // for TriggerSchema, ActionPtr
#include <cloe/utility/inja.hpp> // for inja

namespace vtd {

cloe::TriggerSchema ScpActionFactory::schema() const {
  using namespace cloe;
  return TriggerSchema{
    this->name(),
    this->description(),
    cloe::InlineSchema("template reference as defined in scp_actions configuration"),
    schema::Variant{
      Schema{
        {"xml", make_prototype<std::string>("raw SCP text to send").require()}
      },
      Schema{
        {"template", make_prototype<std::string>("use predefined template").require()},
        {"data", make_prototype<Conf>("map of template parameters")},
      }
    },
  };
}

cloe::ActionPtr ScpActionFactory::make(const cloe::Conf& c) const {
  if (c.has("xml")) {
    auto text = c.get<std::string>("xml");
    return std::make_unique<ScpAction>(name(), scp_client_, text);
  } else {
    auto key = c.get<std::string>("template");
    auto tmpl = predefined_.at(key);
    std::string output;
    if (c.has("data")) {
      cloe::Json data = *c.at("data");
      output = cloe::utility::inja_env().render(tmpl, data);
    } else {
      output = tmpl;
    }
    return std::make_unique<ScpAction>(name(), scp_client_, output);
  }
}

cloe::ActionPtr ScpActionFactory::make(const std::string& s) const {
  return make(cloe::Conf{cloe::Json{
    {"template", s}
  }});
}

} // namespace vtd
