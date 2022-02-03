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

function Card(props) {
  return props.invisible ? null : (
    <div
      className="shadow-sm container pb-2 pt-2 pl-1 pr-1 border white-bg"
      style={{ width: props.width || "auto", wordBreak: "break-word", overflow: "hidden" }}
    >
      {props.children}
    </div>
  );
}

export default Card;
