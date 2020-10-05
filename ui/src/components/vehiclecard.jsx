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
import Card from "./card";
import Label from "./label";
import Chart from "./chart";
import ErrorBoundary from "./errorBoundary";
import { LineChartOutlined } from "@ant-design/icons";

// The VehicleCard component includes:
// * General Information, represented by Label components.
// * A Chart component to represent the speed over time.
class VehicleCard extends Component {
  state = { chartVisible: false, toggleButton: false };

  toggleChart = () => {
    this.setState({
      chartVisible: !this.state.chartVisible,
      toggleButton: !this.state.toggleButton
    });
  };

  render() {
    if (!this.props.vehicle) {
      return <Card>Waiting for vehicle data</Card>;
    }
    const egoSensorName = "cloe::default_ego_sensor";
    const { vehicle, simTime, simulationID } = this.props;
    return (
      <Card>
        <div className="container">
          <div className="row align-items-center">
            <div className="col-3 border-right">
              <h5>{vehicle.name}</h5>
            </div>
            <div className="col-1">
              <LineChartOutlined
                className={`m-2 ${this.state.chartVisible ? "icon-deactive" : "icon"}`}
                style={{ fontSize: "26px" }}
                onClick={() => this.toggleChart()}
              />
            </div>
            <div className="col-2">
              <Label
                label="KM/H"
                value={
                  vehicle.components[egoSensorName].sensed_state.velocity_norm *
                  ((60 * 60) / 1000).toFixed(2)
                }
              />
            </div>
            <div className="col-3">
              <Label
                label="STEERING"
                value={`${vehicle.components[egoSensorName].wheel_steering_angle.toFixed(2)}°`}
              />
            </div>
            <div className="col-3">
              <Label
                label="ACCELERATION"
                value={`${vehicle.components[egoSensorName].sensed_state.acceleration.x.toFixed(
                  2
                )}m/s²`}
              />
            </div>
          </div>
        </div>
        <div id="chart" className={this.state.chartVisible ? "" : "d-none"}>
          <ErrorBoundary>
            <Chart
              className="d-none"
              sourcePaths={["Speed (km/h)"]}
              nextXValue={simTime / 1000}
              nextYValues={[
                vehicle.components[egoSensorName].sensed_state.velocity_norm * ((60 * 60) / 1000)
              ]}
              simulationID={simulationID}
            />
          </ErrorBoundary>
        </div>
      </Card>
    );
  }
}

export default VehicleCard;
