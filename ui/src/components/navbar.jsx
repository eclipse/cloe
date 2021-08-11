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
import React from "react";
import { MoreOutlined } from "@ant-design/icons";
import { Popover } from "antd";
import { copyToClipboard } from "../helpers";
import FileUploader from "./fileUploader";

function NavBar(props) {
  const { connected, version } = props;
  const versionString = version.package_version;
  // Propagate the file to parent app.js.
  const handleFileUpload = (file, content) => {
    props.getSimulationDataFromJSON(file, content);
  };

  return (
    <nav
      className={`navbar navbar-expand-lg navbar-light shadow-sm ${
        connected ? "white-bg" : "red-bg"
      }`}
    >
      <div className="navbar-header">
        <img
          className="navbar-brand"
          src={connected ? require("../images/cloe.svg") : require("../images/cloe_white.svg")}
          alt="Cloe"
        />
        <Popover
          placement="right"
          content={
            <React.Fragment>
              {"Click to copy Cloe version to clipboard:"}
              <br />
              <small>{versionString}</small>
            </React.Fragment>
          }
        >
          <small
            className="font-weight-light text-secondary"
            style={{ cursor: "pointer" }}
            onClick={() => {
              copyToClipboard(versionString);
            }}
          >
            {connected ? version.version : ""}
          </small>
        </Popover>
      </div>
      <FileUploader handleFileUpload={handleFileUpload} connected={connected}></FileUploader>
      <div className="navbar-nav mx-1">
        <div className="float-left my-lg-0 m-auto">
          <MoreOutlined
            className={connected ? "icon" : "icon-white"}
            style={{ fontSize: "26px" }}
            onClick={() => props.toggleSidebar()}
          />
        </div>
      </div>
    </nav>
  );
}

export default NavBar;
