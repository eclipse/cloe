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
import { Popover } from "antd";

// The Timebar component tracks the activity of the controller
// and displays it in an activity bar. It receives the current controller
// state and the current Simulation Time via props, compares it with the
// previous state and renders the bar with the new state.
class Timebar extends Component {
  constructor(props) {
    super(props);
    this.data = {
      nextSectionID: 1,
      startTime: 0,
      sections: [],
      initialStart: true,
      simulationID: null
    };
  }

  updateData = () => {
    const { initialStart, nextSectionID, sections, simulationID } = this.data;
    const { newSimTime, controllerState } = this.props;
    const newSimTimeSeconds = newSimTime / 1000;
    if (newSimTime !== undefined && controllerState !== undefined) {
      this.data.simTime = newSimTimeSeconds;
      // Reset if initial UI start or new simulation started.
      if (initialStart || simulationID !== this.props.simulationID) {
        if (this.props.newSimTime) {
          this.data.initialStart = false;
          this.data.startTime = newSimTimeSeconds;
          this.data.simulationID = this.props.simulationID;
          this.data.sections = [
            {
              key: nextSectionID.toString(),
              state: controllerState,
              start: newSimTimeSeconds,
              end: newSimTimeSeconds
            }
          ];
        }
      } else {
        // After initial UI start, update the current activity item or create a
        // new one if the controller state has changed.
        const updatedSections = sections;
        if (controllerState === sections[sections.length - 1].state) {
          updatedSections[sections.length - 1].end = newSimTimeSeconds;
          this.data.sections = updatedSections;
        } else {
          const startEnd = sections[sections.length - 1].end;
          updatedSections.push({
            key: (nextSectionID + 1).toString(),
            state: controllerState,
            start: startEnd,
            end: newSimTimeSeconds
          });
          this.data.nextSectionID++;
        }
      }
    }
  };

  render() {
    this.updateData();
    const { sections, simTime, startTime } = this.data;
    const backgroundColorTrue = this.props.colorTrue || "#7cb342";
    const backgroundcolorFalse = this.props.colorFalse || "#dc3545";
    return (
      <div className="p-2">
        {sections.map((section) =>
          !(section.start - section.end === 0 && sections.length > 1) ? (
            <Popover
              key={sections.indexOf(section)}
              content={
                <React.Fragment>
                  <h6>{this.getString(section.state)}</h6>
                  {`Start: ${section.start}s`}
                  <br />
                  {`End: ${section.end}s`}
                  <br />
                  {`Duration: ${(section.end - section.start).toFixed(2)}s`}
                  <br />
                </React.Fragment>
              }
            >
              <div
                // eslint-disable-next-line
                style={{
                  backgroundColor: section.state ? backgroundColorTrue : backgroundcolorFalse,
                  height: `${15}px`,
                  float: "left",
                  width: `${((section.end - section.start) / (simTime - startTime) || 1) * 100}%`
                }}
              />
            </Popover>
          ) : null
        )}
        <div className="spacer" style={{ clear: "both" }} />
        <span className="float-left small">{Math.round(startTime)}s</span>
        <span className="float-center small">{this.props.xLabel || "SimTime"}</span>
        <span className="float-right small">{Math.round(simTime)}s</span>
      </div>
    );
  }

  getString = (jsObject) => {
    return jsObject ? jsObject.toString().toUpperCase() : "UNAVAILABLE";
  };
}

export default Timebar;
