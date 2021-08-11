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
export default class RecordService {
  recorder = null;
  stream = null;
  blobs = [];
  constructor(recordSettings) {
    // Read the data in chunks.
    this.timeslice = recordSettings.timeslice;
    this.mimeType = recordSettings.mimeType === void 0 ? "video/webm" : recordSettings.mimeType;
    // Default value is 2.5mbps.
    this.videoBitsPerSecond = 2500000;
    this.canvas = recordSettings.canvas;
  }
  // Start the recording by capturing the canvas stream and collecting the data in the blobs[] array.
  startRecording() {
    this.stream = this.canvas.captureStream();
    this.recorder = new MediaRecorder(this.stream, {
      mimeType: this.mimeType,
      videoBitsPerSecond: this.videoBitsPerSecond
    });
    let self = this;
    this.recorder.ondataavailable = function(event) {
      if (event.data && event.data.size > 0) {
        self.blobs.push(event.data);
      }
    };
    this.recorder.start(this.timeslice);
  }
  // Stop the recording.
  stopRecording() {
    // if recording multiple times, consider choosing right track - getTracks()[?].
    this.stream.getTracks()[0].stop();
    this.recorder.stop();
  }

  // return the blobs[] array.
  getData() {
    let self = this;
    return new Promise((resolve) => {
      resolve(
        new Blob(self.blobs, {
          type: self.mimeType
        })
      );
    });
  }
}
