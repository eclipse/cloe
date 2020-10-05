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
 */
import React, { Component } from "react";
import ToggleSwitch from "./toggleSwitch";

class HMISwitch extends Component {
  state = { switchEnabled: true };

  handleClick = () => {
    if (this.state.switchEnabled) {
      this.setState({ switchEnabled: false }, this.waitForNewState(this.props.switchState));
    }
  };

  waitForNewState = (oldState) => {
    if (this.props.switchState === oldState) {
      window.setTimeout(() => this.waitForNewState(oldState), 100);
    } else {
      this.setState({ switchEnabled: true });
    }
  };

  render() {
    const { targets, sources, toggleSwitch, switchTitle, switchState } = this.props;
    return (
      <React.Fragment>
        <p className="m-0 mt-2 text-center">{switchTitle}</p>
        <ToggleSwitch
          className={`m-auto ${this.state.switchEnabled ? "" : "disabled half-opacity"}`}
          onChange={() => {
            toggleSwitch(targets, sources);
            this.handleClick();
          }}
          checked={switchState || false}
        />
      </React.Fragment>
    );
  }
}

export default HMISwitch;
