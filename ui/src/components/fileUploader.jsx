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
import React from "react";
import { UploadOutlined } from "@ant-design/icons";

const FileUploader = (props) => {
  const { connected, supportedFileTypes } = props;
  const hiddenFileInput = React.useRef(null);

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
    props.handleFileUpload(fileUploaded);
    event.target.value = "";
  };
  return (
    <>
      <input
        type="file"
        ref={hiddenFileInput}
        onChange={handleChange}
        style={{ display: "none" }}
        accept={supportedFileTypes}
      />
      <div className="mr-2 my-lg-0">
        <UploadOutlined
          className={connected ? "icon" : "icon-white"}
          style={{ fontSize: "26px" }}
          onClick={(e) => {
            handleClick(e);
          }}
        />
      </div>
    </>
  );
};
export default FileUploader;
