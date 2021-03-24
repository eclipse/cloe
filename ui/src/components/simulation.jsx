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
import axios from "axios";
import Card from "./card";
import Label from "./label";
import Slider from "react-rangeslider";

// The Simulation component displays general Information about the simulation.
// Each Information is represented by a Label component.
// Aside from that, the Simulation component includes triggers to set the simulation speed.
class Simulation extends Component {
  state = {
    simSpeed: 100,
    uuid: undefined
  };

  setInitialSimSpeed = () => {
    // Initial simSpeed has to be fetched from api by parent component,
    // so it is not available at mounting time.
    // Because of that, this function loops until the necessary prop
    // is defined by the parent component.
    if (!this.props.simulation) {
      window.setTimeout(this.setInitialSimSpeed, 100);
    } else {
      this.setState({ simSpeed: this.props.simulation.realtime_factor * 100 });
    }
  };

  handleSimSpeedChange = (value) => {
    this.setState({ simSpeed: value }, () => {
      this.triggerHMI("realtime_factor", this.state.simSpeed / 100);
    });
  };

  triggerHMI = (dest, value) => {
    const data = {
      action: { name: dest, factor: value },
      event: { name: "_loop" },
      visible: false
    };
    axios.post(
      `http://${this.props.host}${this.props.apiPrefix}/triggers/input`,
      JSON.stringify(data)
    );
  };

  render() {
    if (!this.props.simulation) {
      return <Card>Waiting for simulation data</Card>;
    }
    if (this.state.uuid !== this.props.uuid) {
      this.setInitialSimSpeed();
      this.setState({ uuid: this.props.uuid });
    }
    const { simSpeed } = this.state;
    const { time, step, eta } = this.props.simulation;
    return (
      <Card>
        <div className="row align-items-center">
          <div className={`border-right ${eta ? "col-3" : "col-4"}`}>
            <Label label="SIM TIME" value={`${time / 1000}s`} />
          </div>
          {eta ? (
            <div className="col-3 border-right">
              <Label label="PROGRESS" value={`${((time / eta.ms) * 100).toFixed(0)}%`} />
            </div>
          ) : null}

          <div className={`border-right ${eta ? "col-2" : "col-4"}`}>
            <Label label="STEP" value={step} />
          </div>
          <div className="col">
            <div className="row">
              <div className="col-sm">
                <small className="font-weight-light text-secondary">SIM SPEED</small>
              </div>
            </div>
            <div className="row">
              <div className="col-sm">
                <span className="m-2">{(simSpeed / 100).toFixed(1)}x</span>
                <div className="m-2 btn-group">
                  <Slider
                    value={simSpeed}
                    tooltip={false}
                    onChange={this.handleSimSpeedChange}
                    min={10}
                    max={200}
                    step={10}
                  />
                </div>
              </div>
            </div>
          </div>
        </div>
      </Card>
    );
  }
}

export default Simulation;
