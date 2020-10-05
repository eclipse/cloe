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
import Slider from "react-rangeslider";
import { SaveOutlined } from "@ant-design/icons";
import { Drawer, Input } from "antd";

class Sidebar extends Component {
  render() {
    const {
      sideBarOpen,
      closeSidebar,
      handleEnter,
      host,
      initialHost,
      apiPrefix,
      updateHost,
      connected,
      updateInterval,
      handleInterval,
      draggingActivated,
      setUpdateInterval,
      handleDragNDropSwitch
    } = this.props;
    const links = [
      { name: "Website", link: "https://github.com/eclipse/cloe" },
      { name: "Cloe API", link: `http://${host}` },
      {
        name: "Cloe Configuration",
        link: `http://${host}${apiPrefix}/configuration`
      },
      {
        name: "Cloe Repository",
        link: "https://github.com/eclipse/cloe"
      }
    ];
    return (
      <Drawer visible={sideBarOpen} onClose={closeSidebar}>
        <h4 className="border-bottom pb-2">Settings</h4>
        <React.Fragment>
          <h6>Cloe Host</h6>
          <div className="container">
            <div className="row">
              <Input
                placeholder={initialHost}
                suffix={<SaveOutlined onClick={updateHost} style={{ color: "rgba(0,0,0,.25)" }} />}
                id="cloeHost"
                onKeyUp={handleEnter}
              />
            </div>
          </div>
          <br />
          {!connected ? null : (
            <React.Fragment>
              <h6>Update Interval</h6>

              <Slider
                className="float-left mr-2"
                value={updateInterval}
                tooltip={false}
                onChange={(value) => {
                  handleInterval(value);
                }}
                onChangeComplete={() => setUpdateInterval(updateInterval)}
                min={200}
                max={2000}
                step={100}
              />
              <div
                style={{
                  cursor: "default"
                }}
                className="text-secondary"
              >
                {updateInterval}
                ms
              </div>
              <br />
            </React.Fragment>
          )}
          <h6>Component Drag&Drop</h6>
          <ToggleSwitch
            className="float-left mr-2"
            onChange={handleDragNDropSwitch}
            checked={draggingActivated}
          />
        </React.Fragment>
        <div className="link-button" onClick={() => this.props.resetCookies()}>
          ↻ Reset Layout
        </div>
        <br />
        <h4 className="border-bottom pb-2">Links</h4>
        {links.map((link) => (
          <React.Fragment key={link.name}>
            <a className="niceLink mb-2" href={link.link} target="_blank" rel="noopener noreferrer">
              {link.name}
            </a>
            <br />
          </React.Fragment>
        ))}
      </Drawer>
    );
  }
}

export default Sidebar;
