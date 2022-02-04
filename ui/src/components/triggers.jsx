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
import { Element } from "react-scroll";
import {
  CopyOutlined,
  ExportOutlined,
  InfoCircleOutlined,
  WarningOutlined
} from "@ant-design/icons";
import { Icon as LegacyIcon } from "@ant-design/compatible";
import { Input, Button, message, Popover, Select, Tooltip } from "antd";
import { copyToClipboard, download, parseInput, truncString } from "../helpers";
import axios from "axios";
import Ajv from "ajv";

const ajvActions = new Ajv({ allErrors: true });
const ajvEvents = new Ajv({ allErrors: true });

const Option = Select.Option;
const TextArea = Input.TextArea;

// The Triggers component creates a TriggerList component for
// every trigger type it receives via props.
class Triggers extends Component {
  state = {
    historyExpanded: false,
    queueExpanded: false,
    eventArgumentsVisible: false,
    insertTriggerAsJSONView: false,
    triggerToInsert: {}
  };

  componentDidMount() {
    message.config({ top: 10 });
  }

  // Make sure to only re-render if the trigger list has changed.
  shouldComponentUpdate(nextProps, nextState) {
    if (JSON.stringify(this.props.triggers) !== JSON.stringify(nextProps.triggers)) {
      return true;
    }
    if (nextState !== this.state) {
      return true;
    }
    return false;
  }

  insertTrigger = (data) => {
    if (this.props.connected) {
      axios
        .post(
          `http://${this.props.host}${this.props.apiPrefix}/triggers/input`,
          JSON.stringify(data)
        )
        .then(() => {
          message.success("Trigger inserted successfully!");
          this.setState({ triggerToInsert: {} });
          // Hacky: Clear the state of input fields by switching between
          // the insert views, because we don't have direct access to the
          // value prop of the input.
          this.setState(
            { insertTriggerAsJSONView: true },
            function() {
              this.setState({ insertTriggerAsJSONView: false });
            }.bind(this)
          );
        })
        .catch(function(error) {
          console.log(error.response);
          if (error.response.status === 400) {
            message.error(
              `Bad Request (400): Please check trigger! (${error.response.data.error})`
            );
          } else {
            message.error(
              `${error.response.statusText}(${error.response.status}): ${error.response.data.error}`
            );
          }
        });
    }
  };

  removeNameProperty = (key, value) => {
    if (key === "name") {
      return undefined;
    }
    return value;
  };

  render() {
    if (!this.props.triggers || !(this.props.triggers || {}).queue) {
      return <Card>Waiting for trigger data</Card>;
    }
    const { queue, history } = this.props.triggers;
    const { historyExpanded, queueExpanded, insertTriggerAsJSONView } = this.state;

    // Collect all triggers of each type into one array.
    const sortedQueue = [];
    for (const typeIndex in queue) {
      if (queue[typeIndex] !== null) {
        queue[typeIndex].forEach((trigger) => {
          sortedQueue.push(trigger);
        });
      }
    }

    ajvActions.removeSchema();
    ajvEvents.removeSchema();

    return (
      <Card>
        <div className="row ml-2 mt-2 mb-2 no-gutters">
          <div className="col-4 text-secondary text-left">
            Trigger History
            {this.toggleTriggerList(historyExpanded, "historyExpanded", history.length)}
          </div>
          <div className="col-5"></div>
          {this.exportTriggersJson(history)}
        </div>
        {this.renderTriggerList(
          historyExpanded,
          history.length,
          this.getJsxTriggerHistory(historyExpanded, history)
        )}
        <br />
        <h6 className="ml-2 text-secondary" style={{ textAlign: "left" }}>
          Trigger Queue
          {this.toggleTriggerList(queueExpanded, "queueExpanded", sortedQueue.length)}
        </h6>
        {this.renderTriggerList(
          queueExpanded,
          sortedQueue.length,
          this.getJsxTriggerQueue(queueExpanded, sortedQueue)
        )}
        <br />
        <div className="row ml-2 mt-2 mb-2 no-gutters">
          <div className="col-4 text-secondary text-left">Insert Trigger</div>
          <div className="col-5"></div>
          <div className="col-3 pr-2">
            <small
              className="link-button"
              onClick={() => {
                this.setState({
                  insertTriggerAsJSONView: !insertTriggerAsJSONView,
                  triggerToInsert: {}
                });
              }}
            >
              <div className="float-right ml-1">JSON</div>
              <LegacyIcon
                className="float-right"
                style={{ fontSize: "17px" }}
                type="edit"
                theme={insertTriggerAsJSONView ? "filled" : "outlined"}
              />
            </small>
          </div>
        </div>
        {insertTriggerAsJSONView ? this.insertTriggerAsJSON() : this.insertTriggerFromMenu()}
      </Card>
    );
  }

  getJsxTriggerHistory = (isExpanded, triggerHistory) => {
    return (
      <div className={isExpanded || triggerHistory.length <= 4 ? "border-top border-bottom" : ""}>
        {triggerHistory.map((trigger) => (
          <div
            className={`row pt-1 pb-1 align-items-center triggerList ${
              triggerHistory.indexOf(trigger) % 2 === 0 ? "" : "bg-light"
            }`}
            key={triggerHistory.indexOf(trigger).toString()}
          >
            <div className="col-2">
              <span className="float-left">{trigger.event.name}</span>
            </div>
            <div className="col-3">
              <small className="float-left font-weight-light text-info">
                {truncString(
                  JSON.stringify(trigger.event, this.removeNameProperty).slice(1, -1),
                  17
                )}
              </small>
            </div>
            <div className="col-2">
              <span className="float-left">{trigger.action.name}</span>
            </div>
            <div className="col-3">
              <small className="float-left font-weight-light text-info">
                {truncString(
                  JSON.stringify(trigger.action, this.removeNameProperty).slice(1, -1),
                  17
                )}
              </small>
            </div>
            <div className="col copy">
              <Popover
                content={
                  <React.Fragment>
                    <pre>{JSON.stringify(trigger, null, 2)}</pre>
                  </React.Fragment>
                }
              >
                <small
                  className="icon-clickable link-button mr-4"
                  onClick={() => {
                    copyToClipboard(JSON.stringify(trigger), "Copied trigger to clipboard!");
                  }}
                >
                  <CopyOutlined className="mr-1" />
                  Copy
                </small>
              </Popover>
            </div>
          </div>
        ))}
      </div>
    );
  };

  getJsxTriggerQueue = (isExpanded, triggerQueue) => {
    return (
      <div className={isExpanded || triggerQueue.length <= 4 ? "border-top border-bottom" : ""}>
        {triggerQueue.map((trigger) => (
          <div
            className={`row pt-1 pb-1 align-items-center ${
              triggerQueue.indexOf(trigger) % 2 === 0 ? "" : "bg-light"
            }`}
            key={triggerQueue.indexOf(trigger).toString()}
          >
            <div className="col-2">
              <span className="float-left">{trigger.event.name}</span>
            </div>
            <div className="col-3">
              <small className="float-left font-weight-light text-info">
                {truncString(
                  JSON.stringify(trigger.event, this.removeNameProperty).slice(1, -1),
                  17
                )}
              </small>
            </div>
            <div className="col-3">
              <span className="float-left">{trigger.action.name}</span>
            </div>
            <div className="col-3">
              <small className="float-left font-weight-light text-info">
                {truncString(
                  JSON.stringify(trigger.action, this.removeNameProperty).slice(1, -1),
                  35
                )}
              </small>
            </div>
          </div>
        ))}
      </div>
    );
  };

  insertTriggerFromMenu = () => {
    const { triggerActions, triggerEvents } = this.props;
    const { triggerToInsert } = this.state;

    // Make actions and events available for the input fields.
    const actions = this.getActions(triggerActions);
    const events = this.getEvents(triggerEvents);

    // Validate trigger with JSON schema.
    if (triggerToInsert.action) {
      ajvActions.validate(triggerToInsert.action.name, triggerToInsert.action);
    }
    if (triggerToInsert.event) {
      ajvEvents.validate(triggerToInsert.event.name, triggerToInsert.event);
    }
    const triggerIsValid = !ajvActions.errors && !ajvEvents.errors ? true : false;
    const triggerErrors = {
      events: ajvEvents.errorsText(),
      actions: ajvActions.errorsText()
    };

    // If an action/event is selected in the UI, make the action properties
    // available for the corresponding input fields.
    const actionFields = this.getActionProps(triggerToInsert, triggerActions);
    const eventFields = this.getEventProps(triggerToInsert, triggerEvents);

    return (
      <div className="container">
        {this.selectEvent(triggerToInsert, events)}
        {this.annotateEventFieldInput(triggerToInsert, eventFields)}
        {this.selectAction(triggerToInsert, actions)}
        {this.annotateActionFieldInput(triggerToInsert, actionFields)}
        <div
          className="row"
          style={{
            marginTop: "5px"
          }}
        >
          {this.validateTriggerFromMenu(triggerToInsert, triggerIsValid, triggerErrors)}
          {this.insertTriggerButton(triggerToInsert, triggerIsValid)}
        </div>
      </div>
    );
  };

  getActions = (triggerActions) => {
    const actions = [];
    for (const action in triggerActions) {
      actions.push({
        label: action,
        value: action,
        description: triggerActions[action].description
      });
      ajvActions.addSchema(triggerActions[action], action);
    }
    return actions;
  };

  getActionProps = (triggerToInsert, triggerActions) => {
    const actionFields = [];
    if (triggerToInsert.action) {
      for (const actionField in triggerActions[triggerToInsert.action.name].properties) {
        const currentActionField =
          triggerActions[triggerToInsert.action.name].properties[actionField];
        if (actionField !== "name") {
          actionFields.push({
            label: actionField,
            description: currentActionField.description,
            type: currentActionField.type
          });
        }
      }
    }
    return actionFields;
  };

  getEvents = (triggerEvents) => {
    const events = [];
    for (const event in triggerEvents) {
      events.push({
        label: event,
        value: event,
        description: triggerEvents[event].description
      });
      ajvEvents.addSchema(triggerEvents[event], event);
    }
    return events;
  };

  getEventProps = (triggerToInsert, triggerEvents) => {
    const eventFields = [];
    if (triggerToInsert.event) {
      for (const eventField in triggerEvents[triggerToInsert.event.name].properties) {
        const currentEventField = triggerEvents[triggerToInsert.event.name].properties[eventField];
        if (eventField !== "name") {
          eventFields.push({
            label: eventField,
            required: currentEventField.required,
            description: currentEventField.description,
            type: currentEventField.type
          });
        }
      }
    }
    return eventFields;
  };

  selectEvent = (triggerToInsert, events) => {
    return (
      <div className="row mt-2">
        <div className="col">
          <Select
            style={{ width: "100%" }}
            placeholder="Event"
            allowClear
            onChange={(value) => {
              const trigger = { ...triggerToInsert };
              trigger.event = { name: value };
              if (!value) {
                delete trigger.event;
              }
              this.setState({ triggerToInsert: trigger });
            }}
          >
            {events.map((event) => (
              <Option key={event.value} value={event.value}>
                {event.label}
                <small className="ml-2 mr-2 text-secondary mr-4">{event.description}</small>
              </Option>
            ))}
          </Select>
        </div>
      </div>
    );
  };

  annotateEventFieldInput = (triggerToInsert, eventFields) => {
    if (eventFields.length) {
      return (
        <div>
          {eventFields.map((eventField) => (
            <div key={eventField.label} className="row mt-2">
              <div className="col-2">
                <div>{eventField.label}</div>
              </div>
              <div className="col-10">
                <Input
                  id={eventField.label}
                  suffix={
                    <Tooltip title={`${eventField.type}: ${eventField.description}`}>
                      <InfoCircleOutlined style={{ color: "rgba(0,0,0,.45)" }} />
                    </Tooltip>
                  }
                  allowClear={true}
                  onChange={() => {
                    const trigger = { ...triggerToInsert };
                    const argValue = document.getElementById(eventField.label).value;
                    trigger.event[eventField.label] = parseInput(argValue);
                    if (!argValue) {
                      delete trigger.event[eventField.label];
                    }
                    this.setState({ triggerToInsert: trigger });
                  }}
                />
              </div>
            </div>
          ))}
          <br />
        </div>
      );
    } else {
      return null;
    }
  };

  selectAction = (triggerToInsert, actions) => {
    return (
      <div className="row mt-2">
        <div className="col">
          <Select
            style={{ width: "100%" }}
            placeholder="Action"
            allowClear
            onChange={(value) => {
              const trigger = { ...triggerToInsert };
              trigger.action = { name: value };
              if (!value) {
                delete trigger.action;
              }
              this.setState({ triggerToInsert: trigger });
            }}
          >
            {actions.map((actionType) => (
              <Option key={actionType.value} value={actionType.value}>
                {actionType.label}
                <small className="ml-2 mr-2 text-secondary mr-4">{actionType.description}</small>
              </Option>
            ))}
          </Select>
        </div>
      </div>
    );
  };

  annotateActionFieldInput = (triggerToInsert, actionFields) => {
    if (actionFields.length) {
      return (
        <div>
          {actionFields.map((actionField) => (
            <div key={actionField.label} className="row mt-2">
              <div className="col-2">{actionField.label}</div>
              <div className="col-10">
                <Input
                  id={actionField.label}
                  suffix={
                    <Tooltip title={`${actionField.type}: ${actionField.description}`}>
                      <InfoCircleOutlined style={{ color: "rgba(0,0,0,.45)" }} />
                    </Tooltip>
                  }
                  allowClear={true}
                  onChange={() => {
                    const trigger = { ...triggerToInsert };
                    const argValue = document.getElementById(actionField.label).value;
                    trigger.action[actionField.label] = parseInput(argValue);
                    if (!argValue) {
                      delete trigger.action[actionField.label];
                    }
                    this.setState({ triggerToInsert: trigger });
                  }}
                />
              </div>
            </div>
          ))}
        </div>
      );
    } else {
      return null;
    }
  };

  validateTriggerFromMenu = (triggerToInsert, triggerIsValid, triggerErrors) => {
    // Issue warnings on incomplete/incorrect trigger input.
    const annotateTriggerInput = triggerIsValid ? null : (
      <Popover
        content={
          <React.Fragment>
            Events:
            <br />
            <small>
              {triggerErrors.events.replace("data", "").replace("No errors", "Required")}
            </small>
            <br />
            Actions:
            <br />
            <small>
              {triggerErrors.actions.replace("data", "").replace("No errors", "Required")}
            </small>
          </React.Fragment>
        }
      >
        <WarningOutlined
          style={{
            marginLeft: "5px",
            color: "red",
            verticalAlign: "middle"
          }}
        />
      </Popover>
    );
    // Render warnings, if applicable.
    return (
      <div className="col">
        {Object.keys(triggerToInsert).length !== 0 ? (
          <div className="float-right mt-2 mb-2 text-break">
            <small className="font-weight-light text-secondary">
              {JSON.stringify(triggerToInsert)}
            </small>
            {annotateTriggerInput}
          </div>
        ) : null}
      </div>
    );
  };

  insertTriggerButton = (triggerToInsert, triggerIsValid) => {
    return (
      <div className="col-3">
        <Button
          block
          className="mt-2 float-right"
          disabled={
            !this.props.connected |
            !triggerIsValid |
            !Object.keys(triggerToInsert).includes("event") |
            !Object.keys(triggerToInsert).includes("action")
          }
          type="default"
          onClick={() => {
            this.insertTrigger(triggerToInsert);
          }}
        >
          Insert Trigger
        </Button>
      </div>
    );
  };

  insertTriggerAsJSON = () => {
    return (
      <div className="container">
        <div className="row">
          <div className="col">
            <TextArea placeholder="Trigger in JSON format" id="jsonTextArea" rows={3} />
            <Button
              className="mt-2"
              type="default"
              disabled={!this.props.connected}
              onClick={() => {
                let data = {};
                try {
                  data = JSON.parse(document.getElementById("jsonTextArea").value);
                } catch (e) {
                  alert("please provide valid JSON as Argument");
                  return null;
                }
                this.insertTrigger(data);
                return true;
              }}
            >
              Insert Trigger
            </Button>
          </div>
        </div>
      </div>
    );
  };

  toggleTriggerList = (expanded, name, nTrigger) => {
    if (nTrigger) {
      return (
        <LegacyIcon
          className="fa-sm ml-2 icon-clickable"
          type={expanded ? "up" : "down"}
          onClick={() => {
            this.setState({ [name]: !expanded });
          }}
        />
      );
    } else {
      return " is empty";
    }
  };

  exportTriggersJson = (triggerHistory) => {
    return (
      <div className="col-3 pr-2">
        <small
          className="link-button"
          onClick={() => {
            download(
              `data:text/plain;charset=utf-8,${encodeURIComponent(JSON.stringify(triggerHistory))}`,
              "triggers.json"
            );
          }}
        >
          <div className="float-right ml-1">Export</div>
          <ExportOutlined className="float-right" style={{ fontSize: "17px" }} />
        </small>
      </div>
    );
  };

  renderTriggerList = (expanded, nTrigger, triggerList) => {
    if (nTrigger) {
      return (
        <div className="container">
          {this.getTriggerFormat()}
          {this.getTriggerList(expanded, nTrigger, triggerList)}
        </div>
      );
    } else {
      return null;
    }
  };

  getTriggerFormat = () => {
    return (
      <div className="row">
        <div className="col-2" style={{ textAlign: "left" }}>
          <small className="font-weight-light text-secondary">EVENT</small>
        </div>
        <div className="col-3" />
        <div className="col" style={{ textAlign: "left" }}>
          <small className="font-weight-light text-secondary">ACTION</small>
        </div>
      </div>
    );
  };

  getTriggerList = (expanded, nTrigger, triggerList) => {
    if (expanded || nTrigger <= 4) {
      return triggerList;
    } else {
      return this.createScrollBox(triggerList);
    }
  };

  createScrollBox = (element) => {
    return (
      <Element
        className="border-top border-bottom"
        id="historyElement"
        style={{
          position: "relative",
          height: "130px",
          overflow: "scroll",
          overflowX: "hidden"
        }}
      >
        {element}
      </Element>
    );
  };
}

export default Triggers;
