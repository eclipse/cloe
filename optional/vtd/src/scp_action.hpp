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
 * \file scp_action.hpp
 * \see  scp_action.cpp
 */

#pragma once

#include <memory> // for make_unique<>

#include <cloe/trigger.hpp> // for Action, ActionFactory

#include "scp_transceiver.hpp" // for ScpTransceiver

namespace vtd {

class ScpAction : public cloe::Action {
 public:
  ScpAction(const std::string& name, std::shared_ptr<ScpTransceiver> scp_client, const std::string& msg)
    : cloe::Action(name), client_(scp_client), xml_(msg) {}
  cloe::ActionPtr clone() const override { return std::make_unique<ScpAction>(name(), client_, xml_); }
  cloe::CallbackResult operator()(const cloe::Sync&, cloe::TriggerRegistrar&) override {
    logger()->info("Sending SCP message: {}", xml_);
    client_->send(xml_);
    return cloe::CallbackResult::Ok;
  }
  bool is_significant() const override { return false; }

 protected:
  void to_json(cloe::Json& j) const override {
    j = cloe::Json{
        {"xml", xml_},
    };
  }

 private:
  std::shared_ptr<ScpTransceiver> client_;
  std::string xml_;
};

class ScpActionFactory : public cloe::ActionFactory {
 public:
  using ActionType = ScpAction;
  ScpActionFactory(
    std::shared_ptr<ScpTransceiver> scp_client,
    const std::map<std::string, std::string>& predefined) :
    cloe::ActionFactory("scp", "send an SCP message to the VTD server"),
    scp_client_(scp_client),
    predefined_(predefined) {}
  cloe::TriggerSchema schema() const override;

  /**
   * Takes input like:
   *
   *    {
   *      name: "vtd/scp"
   *      xml: "<SimCtrl><[[cmd]]/><SimCtrl>"
   *    }
   *
   * or:
   *
   *    {
   *      name: "scp"
   *      template: "simctrl"
   *      data:
   *        cmd: "Stop"
   *    }
   */
  cloe::ActionPtr make(const cloe::Conf& c) const override;

  /**
   * Takes template reference without data.
   */
  cloe::ActionPtr make(const std::string& s) const override;

 private:
  std::shared_ptr<ScpTransceiver>  scp_client_;
  std::map<std::string, std::string> predefined_;
};

} // namespace vtd
