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
import {
  UndoOutlined,
  PlayCircleOutlined,
  PauseOutlined,
  FastBackwardOutlined,
  FastForwardOutlined,
  VideoCameraOutlined
} from "@ant-design/icons";
import { Spin } from "antd";
import { STREAMINGTYPES, REPLAYSTATES } from "../enums";

function ReplayControls(props) {
  const { streamingType, replayState, isRecording, isBuffering } = props;

  const handleToggleReplay = (isRunning) => {
    props.toggleReplay(isRunning);
  };

  const handleFastForward = () => {
    props.fastForward();
  };

  const handleRewind = () => {
    props.rewind();
  };

  const handleRecordCanvas = () => {
    props.recordCanvas();
  };

  const isReplayType = (type) => {
    return type === STREAMINGTYPES.REPLAY ? true : false;
  };

  return (
    <React.Fragment>
      {isReplayType(streamingType) && (
        <span
          className="badge badge-info p-2 position-absolute"
          style={{ right: "25px", top: "15px", textAlign: "left" }}
        >
          Replay Mode
        </span>
      )}
      {
        <div
          className="m-0 mt-2 text-light mr-auto ml-auto left position-absolute"
          style={{
            left: "0",
            right: "0",
            textAlign: "center",
            transform: "translate(0px, 320px)"
          }}
        >
          {isReplayType(streamingType) && (
            <FastBackwardOutlined
              className="m-2 icon-white"
              style={{ color: "#2bbbad", fontSize: "16px" }}
              onClick={() => handleRewind()}
            />
          )}
          {isReplayType(streamingType) && replayState === REPLAYSTATES.PAUSED && (
            <PlayCircleOutlined
              className={`m-2 ${isBuffering ? "icon-invisible" : "icon-white"}`}
              style={{ color: "#2bbbad", fontSize: "16px" }}
              onClick={() => handleToggleReplay(REPLAYSTATES.STARTED)}
            />
          )}
          {isReplayType(streamingType) && replayState === REPLAYSTATES.STARTED && (
            <PauseOutlined
              className="m-2 icon-white"
              style={{ color: "#2bbbad", fontSize: "16px" }}
              onClick={() => handleToggleReplay(REPLAYSTATES.PAUSED)}
            />
          )}
          {isReplayType(streamingType) && replayState === REPLAYSTATES.FINISHED && (
            <UndoOutlined
              className="m-2 icon-white"
              style={{ color: "#2bbbad", fontSize: "16px" }}
              onClick={() => handleToggleReplay(REPLAYSTATES.FINISHED)}
            />
          )}
          {isReplayType(streamingType) && (
            <FastForwardOutlined
              className="m-2 icon-white"
              style={{ color: "#2bbbad", fontSize: "16px" }}
              onClick={() => handleFastForward()}
            />
          )}
          <VideoCameraOutlined
            className="m-2 icon-white"
            style={{ color: "#2bbbad", fontSize: "16px" }}
            onClick={() => handleRecordCanvas()}
          />
          <div
            style={{
              width: "10px",
              height: "8px",
              display: "inline-block"
            }}
          >
            <div className={`${isRecording ? "rec-icon-pending" : "invisible"}`}></div>
          </div>
        </div>
      }
      {isReplayType(streamingType) && replayState === REPLAYSTATES.PAUSED && isBuffering && (
        <Spin className="custom-spinner" tip="Loading..." />
      )}
    </React.Fragment>
  );
}

export default ReplayControls;
