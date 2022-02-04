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
import React, { useState } from "react";
import { MoreOutlined } from "@ant-design/icons";
import { Popover } from "antd";
import { copyToClipboard } from "../helpers";
import FileUploader from "./fileUploader";
import ErrorMessage from "./errorMessage";
import axios from "axios";

function NavBar(props) {
  const { connected, version, hostname, webserverPort } = props;
  const versionString = version.package_version;
  const host = `http://${hostname}:${webserverPort}`;
  const [state, setState] = useState({ showErrorMessage: false, fileType: "" });
  // Propagate the file to parent app.js.
  const handleFileUpload = async (file) => {
    let reader = new FileReader();
    if (file.type === "application/json") {
      props.setupReplayEnvironment();
      reader.onload = function(e) {
        props.getSimulationDataFromJSON(file, e.target.result);
      };
      reader.readAsText(file);
    } else if (file.type === "application/x-gzip") {
      const fd = new FormData();
      fd.append("uploaded_file", file);
      const config = {
        headers: {
          "Content-Type": "multipart/form-data"
        }
      };
      // Send data to server to decompress gz file server side .
      try {
        props.setupReplayEnvironment();
        const response = await axios.post(host + "/decompress-gzips", fd, config);
        axios.delete(host + "/delete-gzips/" + file.name);
        props.getSimulationDataFromJSON(file, response.data);
      } catch (error) {
        console.log(error);
      }
    } else {
      // Show error message for 5 seconds.
      setState({
        ...state,
        showErrorMessage: true,
        fileType: file.type
      });
      setTimeout(() => {
        setState({
          ...state,
          showErrorMessage: false
        });
      }, 5000);
    }
  };

  return (
    <>
      {state.showErrorMessage && (
        <ErrorMessage
          errorMessage={" not supported. Try .json or .json.gz files"}
          fileType={state.fileType}
        ></ErrorMessage>
      )}
      <nav
        className={`navbar navbar-expand-lg navbar-light shadow-sm ${
          connected ? "white-bg" : "red-bg"
        }`}
        style={{ justifyContent: "space-between" }}
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
        <div className="navbar-nav mx-1">
          <FileUploader
            handleFileUpload={handleFileUpload}
            connected={connected}
            supportedFileTypes={".json, .json.gz"}
          ></FileUploader>
          <div className="float-left my-lg-0 m-auto">
            <MoreOutlined
              className={connected ? "icon" : "icon-white"}
              style={{ fontSize: "26px" }}
              onClick={() => props.toggleSidebar()}
            />
          </div>
        </div>
      </nav>
    </>
  );
}

export default NavBar;
