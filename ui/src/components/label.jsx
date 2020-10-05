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

// The Label component represents one single key-value pair.
// It displays a value with it's key (label).
function Label(props) {
  const { label, value, led = false } = props;
  return (
    <div className="pt-2 pb-2 container">
      <div className="row">
        <div className="col-sm">
          <small className="font-weight-light text-secondary">{label}</small>
        </div>
      </div>
      <div className="row">
        <div className="col-sm">
          <span>
            {typeof value === "boolean" && led ? (
              <span className={`mr-2 ${value ? "dotTrue" : "dotFalse"}`} />
            ) : (
              ""
            )}

            {renderValue(value)}
          </span>
        </div>
      </div>
    </div>
  );
}

function renderValue(value) {
  switch (typeof value) {
    case "boolean":
      return value.toString().toUpperCase();
    case "number":
      if (value % Math.round(value) === 0) {
        return value;
      } else {
        return parseFloat(value).toFixed(2);
      }
    default:
      return value || "undefined";
  }
}

export default Label;
