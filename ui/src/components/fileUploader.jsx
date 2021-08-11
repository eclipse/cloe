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
import React, { useState } from "react";
import { UploadOutlined } from "@ant-design/icons";
import pako from "pako";
import ErrorMessage from "./errorMessage";

const FileUploader = (props) => {
  const { connected } = props;
  const hiddenFileInput = React.useRef(null);
  const [state, setState] = useState({ showErrorMessage: false, fileType: "" });
  // This method triggers the handleChange() event.
  const handleClick = (event) => {
    hiddenFileInput.current.click();
  };
  // This method is processing the file with the FileReader API
  // and sends the content to the parent method.
  const handleChange = (event) => {
    const fileUploaded = event.target.files[0];
    if (fileUploaded === undefined) {
      return;
    }
    let reader = new FileReader();
    if (fileUploaded.type === "application/json") {
      reader.onload = function(e) {
        props.handleFileUpload(fileUploaded, e.target.result);
      };
      reader.readAsText(fileUploaded);
    } else if (
      fileUploaded.type === "application/x-gzip" ||
      fileUploaded.type === "application/gzip"
    ) {
      reader.onload = function(e) {
        let result = pako.inflate(e.target.result, { to: "string" });
        props.handleFileUpload(fileUploaded, result);
      };
      reader.readAsArrayBuffer(fileUploaded);
    } else {
      // Show error message for 5 seconds.
      setState({
        ...state,
        showErrorMessage: true,
        fileType: fileUploaded.type
      });
      setTimeout(() => {
        setState({
          ...state,
          showErrorMessage: false
        });
      }, 5000);
    }
    event.target.value = "";
  };
  return (
    <>
      {state.showErrorMessage && (
        <ErrorMessage
          errorMessage={" not supported. Try .json or .json.gz files"}
          fileType={state.fileType}
        ></ErrorMessage>
      )}
      <input
        type="file"
        ref={hiddenFileInput}
        onChange={handleChange}
        style={{ display: "none" }}
        accept=".json, .json.gz"
      />
      <div className="navbar-nav ml-auto mr-2">
        <div className="float-left my-lg-0 m-auto">
          <UploadOutlined
            className={connected ? "icon" : "icon-white"}
            style={{ fontSize: "26px" }}
            onClick={(e) => {
              handleClick(e);
            }}
          />
        </div>
      </div>
    </>
  );
};
export default FileUploader;
