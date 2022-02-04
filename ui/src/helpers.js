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
import { message } from "antd";
message.config({ top: 10 });

export function copyToClipboard(copyString, messageString) {
  var text = document.createElement("textarea");
  text.value = copyString;
  text.setAttribute("readonly", "");
  text.style = { position: "absolute", left: "-9999px" };
  document.body.appendChild(text);
  text.select();
  document.execCommand("copy");
  message.success(`${messageString || copyString} copied to clipboard`);
  document.body.removeChild(text);
}

export function download(data, filename) {
  var el = document.createElement("a");
  el.setAttribute("href", data);
  el.setAttribute("download", filename);

  if (document.createEvent) {
    var event = document.createEvent("MouseEvents");
    event.initEvent("click", true, true);
    el.dispatchEvent(event);
  } else {
    el.click();
  }
}

export function parseBool(boolString) {
  return boolString.toLowerCase() === "true";
}

export function truncString(string, length) {
  return string.length > length ? `${string.substr(0, length - 1)}...` : string;
}

export function parseInput(inputString) {
  let returnValue = inputString;
  try {
    returnValue = JSON.parse(returnValue);
  } catch {
    returnValue =
      returnValue.toLowerCase() === "false"
        ? false
        : parseBool(returnValue) || parseFloat(returnValue) || returnValue;
  }
  return returnValue;
}
