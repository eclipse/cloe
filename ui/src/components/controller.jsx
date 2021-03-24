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
import Timebar from "./timebar";
import Chart from "./chart";
import HMISwitch from "./hmiSwitch";
import Slider from "react-rangeslider";
import hash from "object-hash";
import jp from "jsonpath";
import { Icon as LegacyIcon } from "@ant-design/compatible";
import * as mathjs from "mathjs";

class Controller extends Component {
  constructor(props) {
    super(props);
    this.uiSpecificationLoaded = false;
    this.paths = null;
    this.layout = null;
    this.elements = null;
    this.sources = {};
    this.groupsVisibility = props.layout;
    this.uiEnabled = true;
  }

  render() {
    if (!this.props.controller) {
      return <Card>Waiting for controller data</Card>;
    }
    if (!this.uiSpecificationLoaded) {
      // Load the ui configuration from the cloe api,
      // this should be done just once.
      if (this.uiEnabled) {
        this.fetchUIConfiguration();
        return <Card>Loading UI specification</Card>;
      } else {
        return <React.Fragment />;
      }
    } else {
      // Fetch all defined API endpoints to update the data, generate the
      // elements out of it.
      this.updateData();
      this.generateJSXComponent();
      return (
        <React.Fragment>
          {// Render the layout out of the single elements
          // and the layout definition.
          this.renderController(this.layout)}
        </React.Fragment>
      );
    }
  }

  fetchUIConfiguration = () => {
    if (this.props.connected && this.props.host !== "" && this.props.controller.endpointBase) {
      axios
        .get(`http://${this.props.host}${this.props.controller.endpointBase}/ui`)
        .then((response) => response.data)
        .then((response) => {
          this.paths = response.controller.paths;
          this.layout = response.controller.layout;
          this.title = response.controller.title;
          this.action = response.controller.action;
          this.elements = this.generateElements(response.controller.elements);
          this.uiSpecificationLoaded = true;
        })
        .catch(() => {
          // No UI enabled.
          this.uiEnabled = false;
        });
    }
  };

  generateElements = (specifiedElements) => {
    const elements = [];
    for (const index in specifiedElements) {
      elements.push(this.generateElement(specifiedElements[index]));
    }
    return elements;
  };

  updateData = () => {
    if (this.props.connected && this.props.host !== "") {
      for (const endpoint in this.sources) {
        const fetchEndpoint = this.resolveEndpoint(endpoint);
        axios
          .get(`http://${this.props.host}${fetchEndpoint}`)
          .then((response) => response.data)
          .then((response) => {
            for (const path in this.sources[endpoint]) {
              this.sources[endpoint][path] = jp.value(response, path);
            }
          });
      }
    }
  };

  triggerHMIButton = (actions, value) => {
    if (this.props.connected) {
      for (const actionIndex in actions) {
        let action = actions[actionIndex];
        action.name = action.name || this.action;
        action = JSON.stringify(action);
        action = action.replace("$CONTROLLER", this.props.controller.id);
        action = action.replace('"$STATE"', value);
        action = JSON.parse(action);
        const data = {
          event: "next",
          action: action
        };
        axios.post(
          `http://${this.props.host}${this.props.apiPrefix}/triggers/input`,
          JSON.stringify(data)
        );
      }
    }
  };

  triggerHMISwitch = (actions, sources) => {
    if (this.props.connected) {
      for (const actionIndex in actions) {
        const source = sources[actionIndex];
        const currentState = jp.value(this.sources[source.endpoint], source.path);
        let action = actions[actionIndex];
        action.name = action.name || this.action;
        action = JSON.stringify(action);
        action = action.replace("$CONTROLLER", this.props.controller.id);
        action = action.replace('"$STATE"', !currentState);
        action = action.replace('"$NOT_STATE"', currentState);
        action = JSON.parse(action);

        if (typeof currentState === "boolean") {
          const data = {
            event: "next",
            action: action
          };
          axios.post(
            `http://${this.props.host}${this.props.apiPrefix}/triggers/input`,
            JSON.stringify(data)
          );
        }
      }
    }
  };

  // This function will create objects for each element
  // with the basic information (id, type, title, target, source).
  generateElement = (specifiedElement) => {
    // Add basic information which should be included in every element.
    const element = {
      type: specifiedElement.type,
      id: specifiedElement.id,
      title: specifiedElement.title
    };

    if (specifiedElement.params) {
      element.params = specifiedElement.params;
    }

    // Extract target and source paths/names, if existent.
    if (specifiedElement.action) {
      element.actions = Array.isArray(specifiedElement.action)
        ? specifiedElement.action
        : [specifiedElement.action];
    }
    if (specifiedElement.source) {
      const sources = Array.isArray(specifiedElement.source)
        ? specifiedElement.source
        : [specifiedElement.source];
      element.sources = [];
      for (const sourceIndex in sources) {
        element.sources[sourceIndex] = {
          endpoint: this.getSourceEndpoint(sources[sourceIndex].endpoint),
          path: sources[sourceIndex].path,
          name: sources[sourceIndex].name,
          math: sources[sourceIndex].math
        };

        if (!this.sources[element.sources[sourceIndex].endpoint]) {
          this.sources[element.sources[sourceIndex].endpoint] = {};
        }
        this.sources[element.sources[sourceIndex].endpoint][
          element.sources[sourceIndex].path
        ] = null;
      }
    }
    return element;
  };

  generateJSXComponent = () => {
    for (const index in this.elements) {
      switch (this.elements[index].type) {
        case "button":
          this.elements[index] = this.generateButton(this.elements[index]);
          break;
        case "switch":
          this.elements[index] = this.generateSwitch(this.elements[index]);
          break;
        case "label":
          this.elements[index] = this.generateLabel(this.elements[index]);
          break;
        case "led_label":
          this.elements[index] = this.generateLabel(this.elements[index], true);
          break;
        case "timebar":
          this.elements[index] = this.generateTimebar(this.elements[index]);
          break;
        case "linechart":
          this.elements[index] = this.generateLinechart(this.elements[index]);
          break;
        case "slider":
          this.elements[index] = this.generateSlider(this.elements[index]);
          break;
        default:
          break;
      }
    }
  };
  // These functions will process the individual parameters for
  // eacht element based on it's type.
  generateButton = (buttonElement) => {
    // Set the default style for the button.
    let style = buttonElement.params.default_style;
    // Try to change style in case that the button's source indicates that
    // the alternative style is needed.
    if (buttonElement.sources !== undefined) {
      if (!this.getSourceValue(buttonElement.sources[0])) {
        style = buttonElement.params.alt_style || style;
      }
    }
    // Create react element.
    buttonElement.reactElement = (
      <button
        key={buttonElement.id}
        className={`btn btn-block btn-${style}`}
        onMouseDown={() => this.triggerHMIButton(buttonElement.actions, true)}
        onMouseUp={() => this.triggerHMIButton(buttonElement.actions, false)}
      >
        {buttonElement.title}
      </button>
    );

    return buttonElement;
  };

  generateSwitch = (switchElement) => {
    const switchState = this.getSourceValue(switchElement.sources[0]);
    switchElement.reactElement = (
      <HMISwitch
        key={switchElement.id}
        targets={switchElement.actions}
        sources={switchElement.sources}
        toggleSwitch={this.triggerHMISwitch}
        switchTitle={switchElement.title}
        switchState={switchState}
      />
    );
    return switchElement;
  };

  generateLabel = (labelElement, led = false) => {
    const value = this.getSourceValue(labelElement.sources[0]);
    const title = labelElement.title;
    labelElement.reactElement = (
      <Label key={labelElement.id} label={title} value={value} led={led} />
    );
    return labelElement;
  };

  generateTimebar = (timebarElement) => {
    // Set the triggering state for the timebar.
    const value = this.getSourceValue(timebarElement.sources[0]);
    timebarElement.reactElement = (
      <React.Fragment>
        <h6>{timebarElement.title}</h6>
        <Timebar
          key={timebarElement.id}
          controllerState={value}
          newSimTime={this.props.simTime}
          simulationID={this.props.simulationID}
          colorTrue={timebarElement.params.color_true}
          colorFalse={timebarElement.params.color_false}
          xLabel={timebarElement.params.x_label}
        />
      </React.Fragment>
    );
    return timebarElement;
  };

  generateLinechart = (linechartElement) => {
    const sourcePaths = [];
    const values = [];
    const sources = linechartElement.sources;
    for (const source in sources) {
      sourcePaths.push(sources[source].name);
      values.push(this.getSourceValue(sources[source]));
    }
    linechartElement.reactElement = (
      <React.Fragment>
        <h6>{linechartElement.title}</h6>
        <Chart
          key={linechartElement.id}
          sourcePaths={sourcePaths}
          nextYValues={values}
          nextXValue={this.props.simTime / 1000}
          simulationID={this.props.simulationID}
          colors={linechartElement.params.colors}
        />
      </React.Fragment>
    );
    return linechartElement;
  };

  generateSlider = (sliderElement) => {
    if (sliderElement.value === undefined) {
      sliderElement.value = this.getSourceValue(sliderElement.sources[0]);
    }
    sliderElement.reactElement = (
      <React.Fragment>
        <h6>{sliderElement.title}</h6>
        <Slider
          value={sliderElement.value}
          onChange={(value) => {
            sliderElement.value = value;
            this.triggerHMIButton(sliderElement.actions, value);
            this.forceUpdate();
          }}
          min={sliderElement.params.minValue}
          max={sliderElement.params.maxValue}
          step={sliderElement.params.stepWidth}
        />
      </React.Fragment>
    );
    return sliderElement;
  };

  // Helper functions:
  getSourceEndpoint = (path) => {
    return path || this.paths.source;
  };

  splitArray = (array, rowItems) => {
    const splittedArray = [];
    for (let i = 0; i < array.length; i += rowItems) {
      splittedArray.push(array.slice(i, i + rowItems));
    }
    return splittedArray;
  };

  getElement = (id) => {
    for (const index in this.elements) {
      if (this.elements[index].id === id) {
        return this.elements[index];
      }
    }
    return undefined;
  };

  getSourceValue = (source) => {
    const endpoint = source.endpoint;
    const path = source.path;
    const math = source.math;
    let value = this.sources[endpoint][path];
    if (math !== undefined && typeof value === "number") {
      const scope = {
        x: value
      };
      value = mathjs.evaluate(math, scope);
    }
    return value;
  };

  resolveEndpoint = (endpoint) => {
    // Search for $CONTROLLER variable and replace it with correct endpoint path.
    // Return updated endpoint or the default endpoint (if no endpoint is set).
    return endpoint.split("/")[1] === "$CONTROLLER"
      ? endpoint.replace("/$CONTROLLER", this.props.controller.endpointBase)
      : endpoint;
  };

  // Generates the specified layout.
  renderController = (layout) => {
    const groups = this.getLayoutGroups(layout);
    return (
      <Card>
        <small
          className="font-weight-light text-secondary"
          style={{ position: "absolute", right: "25px" }}
        >
          {this.version || null}
        </small>
        <h5 className="ml-2 text-secondary" style={{ textAlign: "left" }}>
          {this.title}
        </h5>

        {groups.map((group) => (
          <div
            key={hash(group.elements)}
            className={
              groups.indexOf(group) === groups.length - 1
                ? "mt-3 container"
                : "mt-3 container border-bottom"
            }
          >
            {
              <h6
                className="mt-2 text-secondary"
                style={{
                  textAlign: "left",
                  cursor: "pointer",
                  display: "table"
                }}
                onClick={() =>
                  this.toggleGroupVisibility(hash(group.elements), group.visibility_toggle)
                }
              >
                {`${group.name || ""} `}
                <LegacyIcon
                  className="icon-clickable"
                  style={{ verticalAlign: "middle" }}
                  type={this.checkVisibility(group, "down", "up")}
                />
              </h6>
            }
            {group.elements.map((row) => (
              <div
                key={hash(row)}
                className={this.checkVisibility(group, "d-none", "row pb-2 pt-2")}
              >
                {row.map((column) => (
                  <div
                    key={hash({ column: column })}
                    className={
                      (this.getElement(column).params || {}).fitted_width
                        ? "col-auto m-auto"
                        : "col m-auto"
                    }
                  >
                    {this.getElement(column).reactElement}
                  </div>
                ))}
              </div>
            ))}
          </div>
        ))}
      </Card>
    );
  };

  getLayoutGroups = (layout) => {
    const groups = [];
    for (const index in layout.groups) {
      const group = layout.groups[index];
      const numberOfColumns = group.columns;
      const groupedElements = this.splitArray(group.elements, numberOfColumns);
      const numberOfRows = groupedElements.length;
      const newGroup = [];
      newGroup.name = group.name;
      newGroup.visibility_toggle = group.visibility_toggle;
      newGroup.elements = [];
      for (let r = 0; r < numberOfRows; r++) {
        const row = [];
        for (let c = 0; c < numberOfColumns; c++) {
          if (groupedElements[r][c]) {
            row.push(groupedElements[r][c]);
          }
        }
        newGroup.elements.push(row);
      }
      groups.push(newGroup);
    }
    return groups;
  };

  toggleGroupVisibility = (groupHash, defaultVisibility) => {
    switch (this.groupsVisibility[groupHash]) {
      case true:
        this.groupsVisibility[groupHash] = false;
        break;
      case false:
        this.groupsVisibility[groupHash] = true;
        break;
      default:
        this.groupsVisibility[groupHash] =
          defaultVisibility === undefined ? false : !defaultVisibility;
    }
    this.props.saveLayout("controllerLayout", this.groupsVisibility);
    this.forceUpdate();
  };

  checkVisibility = (group, str1, str2) => {
    if (this.groupsVisibility[hash(group.elements)] === undefined) {
      return group.visibility_toggle === false ? str1 : str2;
    } else {
      return this.groupsVisibility[hash(group.elements)] ? str2 : str1;
    }
  };
}

export default Controller;
