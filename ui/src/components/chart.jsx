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
import { Scatter } from "react-chartjs-2";
import "chartjs-plugin-zoom";

// The Chart component creates a chart out of data.
// Every time the props 'nextXValue' and 'nextYValues' are changing,
// the component adds them to it"s data set and renders the new state.
class Chart extends Component {
  constructor(props) {
    super(props);
    this.lastXValue = 0;
    this.initialStart = true;
    this.simulationID = null;
  }

  shouldComponentUpdate(nextProps) {
    this.updateChartData();
    this.refs.chart.chartInstance.chart.update(0);
    return false;
  }

  resetZoom = () => {
    this.refs.chart.chartInstance.resetZoom();
  };

  updateChartData = () => {
    const { nextXValue, nextYValues, sourcePaths, colors, simulationID } = this.props;
    const data = this.refs.chart.chartInstance.chart.data;
    if (this.simulationID !== simulationID) {
      this.simulationID = simulationID;
      this.initialStart = true;
    }
    if (this.initialStart) {
      this.initialStart = false;
      data.datasets = [];
      nextYValues.forEach(() => {
        data.datasets.push({ data: [] });
      });
    }

    for (const index in nextYValues) {
      var dataset = data.datasets[index];
      dataset.label = sourcePaths[index];
      dataset.borderColor = (colors || [])[index] || "black";
      dataset.borderWidth = 1;
      dataset.fill = false;
      dataset.pointRadius = 0;
      dataset.lineTension = 0;
      dataset.showLine = true;
      dataset.data.push({ x: nextXValue, y: nextYValues[index] });
    }
  };

  render() {
    return (
      <div ref="child">
        <small
          className="link-button"
          style={{ position: "absolute", right: "30px" }}
          onClick={() => this.resetZoom()}
        >
          ↻ Reset Zoom
        </small>

        <Scatter
          ref="chart"
          data={{ datasets: [{ data: [0, 0] }] }}
          options={{
            scales: {
              xAxes: [
                {
                  ticks: { autoSkipPadding: 30 },
                  type: "linear",
                  position: "bottom"
                }
              ]
            },
            legend: { labels: { boxWidth: 1 } },
            plugins: {
              zoom: {
                zoom: {
                  enabled: true,
                  drag: true,
                  mode: "x"
                }
              }
            }
          }}
        />
      </div>
    );
  }
}

export default Chart;
