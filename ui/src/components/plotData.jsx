/*
 * Copyright 2021 Robert Bosch GmbH
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
import FileUploader from "./fileUploader";
import $ from "jquery";

class PlotData extends Component {
  constructor(props) {
    super(props);
    this.plotData = {};
    this.iframeData = {};
    this.state = {
      elementID: null,
      uiSpecificationLoaded: false,
      timeMarkersCount: 0
    };
    this.title = null;
  }

  render() {
    if (!this.state.uiSpecificationLoaded) {
      // this should be done just once.
      return (
        <Card>
          <div className="row justify-content-center position-relative">
            plots/reports
            <div className="position-absolute" style={{ right: "30px" }}>
              <FileUploader
                handleFileUpload={this.handleFileUpload}
                connected={true}
                supportedFileTypes={".html"}
              ></FileUploader>
            </div>
          </div>
        </Card>
      );
    } else {
      if (this.props.replayState === 1) {
        this.updateTimeMarkers();
        return (
          <Card>
            <div className="row" style={{ overflow: "scroll" }}>
              {this.title}
              <div className="position-absolute" style={{ right: "30px" }}>
                <FileUploader
                  handleFileUpload={this.handleFileUpload}
                  connected={true}
                  supportedFileTypes={".html"}
                ></FileUploader>
              </div>
              <div className="" style={{ width: "1000px" }}>
                <iframe
                  srcDoc={this.iframeData}
                  id="plotData"
                  title={this.elementID}
                  style={{
                    overflow: "scroll",
                    width: "1000px",
                    height: "500px",
                    border: 0,
                    padding: "15px"
                  }}
                />
              </div>
            </div>
          </Card>
        );
      } else {
        return (
          <Card>
            <div className="row" style={{ overflow: "scroll" }}>
              {this.title}
              <div className="position-absolute" style={{ right: "30px" }}>
                <FileUploader
                  handleFileUpload={this.handleFileUpload}
                  connected={true}
                  supportedFileTypes={".html"}
                ></FileUploader>
              </div>
              <div style={{ width: "1000px" }}>
                <iframe
                  srcDoc={this.iframeData}
                  id="plotData"
                  title={this.elementID}
                  style={{
                    overflow: "scroll",
                    width: "1000px",
                    height: "500px",
                    border: 0,
                    padding: "15px"
                  }}
                />
              </div>
            </div>
          </Card>
        );
      }
    }
  }

  addTimeMarkers = () => {
    if (!$("#plotData").contents().length > 0) {
      return;
    } else if (this.state.timeMarkersCount === 0) {
      let iframeData = $("#plotData").contents()[0].children[0].children[1].children[0].children[2]
        .data;

      let iframeID = $("#plotData").contents()[0].children[0].children[1].children[0].children[2]
        .id;

      let plotlyjs = window[0].Plotly;

      let timeMarkers = [];
      for (let i = 0; i < iframeData.length; i++) {
        if (!Object.prototype.hasOwnProperty.call(iframeData[i], "hoverinfo")) {
          timeMarkers.push({
            legendgroup: iframeData[i].legendgroup,
            line: { color: "rgb(255, 51, 51)", dash: "solid", width: 2, height: 2 },
            mode: "line",
            name: "time",
            type: "scattergl",
            x: [0, 0],
            xaxis: iframeData[i].xaxis,
            y: [0, 0],
            yaxis: iframeData[i].yaxis
          });
        } else {
          continue;
        }
      }
      this.plotData = iframeData.concat(timeMarkers);
      $(
        "#plotData"
      ).contents()[0].children[0].children[1].children[0].children[2].data = this.plotData;
      plotlyjs.redraw(iframeID);
      this.setState({
        elementID: iframeID,
        timeMarkersCount: timeMarkers.length
      });
    }
  };

  updateTimeMarkers = () => {
    let plotlyjs = window[0].Plotly;
    // Reduce update intervall to save resources.
    if ((this.props.simTime / 1000) % 1 === 0) {
      for (
        let i = this.plotData.length - 1;
        i >= this.plotData.length - this.state.timeMarkersCount;
        i--
      ) {
        this.plotData[i].x = [this.props.simTime / 1000, this.props.simTime / 1000];
      }

      $(
        "#plotData"
      ).contents()[0].children[0].children[1].children[0].children[2].data = this.plotData;
      plotlyjs.redraw(this.state.elementID);
    }
  };

  extractDataFromString = (file) => {
    this.iframeData = file;
    this.setState({ uiSpecificationLoaded: true });
    setTimeout(() => {
      this.addTimeMarkers();
    }, 1000);
  };

  handleFileUpload = (file) => {
    let reader = new FileReader();
    if (file.type === "text/html") {
      reader.onload = function(e) {
        this.extractDataFromString(e.target.result);
      }.bind(this);
      reader.readAsText(file);
    } else {
    }
  };
}

export default PlotData;
