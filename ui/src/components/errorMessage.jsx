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

// This component shows different error messages depending on content.
class ErrorMessage extends Component {
  constructor(props) {
    super(props);
    this.fileType = props.fileType || null;
    this.errorMessage = props.errorMessage;
    this.state = {
      showErrorMessage: true
    };
  }

  removeErrorMessage = () => {
    this.setState({ showErrorMessage: false });
  };

  render() {
    if (this.state.showErrorMessage) {
      return (
        <div
          className="alert alert-warning alert-dismissible fade show"
          style={{ position: "absolute", right: "10px", top: "150px", zIndex: "9999" }}
          role="alert"
        >
          <strong>{this.fileType}</strong>
          {this.errorMessage}
          <button
            type="button"
            className="close"
            onClick={(e) => {
              this.removeErrorMessage();
            }}
            aria-label="Close"
          >
            <span aria-hidden="true">&times;</span>
          </button>
        </div>
      );
    } else {
      return <React.Fragment></React.Fragment>;
    }
  }
}

export default ErrorMessage;
