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
/**
 * This is the main file for the cloe webserver.
 * For now everything is located here. API Endpoints and websocket handling
 * could be written in separate files to archieve a better project structure.
 *
 * Further Work:
 *  -API Endpoint to share config file between server/client
 *
 */

// Import libraries.
const express = require("express");
const os = require("os");
const cors = require("cors");
const multer = require("multer");
const fs = require("fs");
const zlib = require("zlib");
const app = express();
const path = require("path");
const JSONStream = require("JSONStream");
// Enable CORS.
// Cross Origin Resource Sharing -> see: https://expressjs.com/en/resources/middleware/cors.html.
// Set credentials to true to allow HTTP Cookies.
app.use(cors({ credentials: true, origin: "*" }));

// Config express webserver.
app.use(express.urlencoded({ extended: true }));
app.use(express.json());
app.use(express.static("uploads"));

const port = process.env.CLOE_UI_WEBSERVER_PORT || 4000;

const FILEPATHROOT = process.env.CLOE_UI_SERVER_ROOT_DIR
  ? process.env.CLOE_UI_SERVER_ROOT_DIR
  : os.homedir() + "/.local/share/cloe";

const FILEDIRECTORY = "replay_data";

// Create directory ./uploads for multer if not existing.
const DIR = "./uploads";

if (!fs.existsSync(DIR)) {
  fs.mkdirSync(DIR);
}

// Configure the multer storage.
const storage = multer.diskStorage({
  destination: function(req, file, cb) {
    cb(null, path.join(__dirname, DIR));
  },
  filename: function(req, file, cb) {
    cb(null, file.originalname);
  }
});

// Define multer function.
const uploadsConfig = multer({ storage });

// The Server will run on 0.0.0.0 and is available for all clients within the network.
const server = app.listen(port, () => console.log(`Listening on port ${port}`));

// Make websocket listen on port.
const io = require("socket.io")(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

io.sockets.on("connection", function(socket) {
  console.log(
    `new Client ${socket.handshake.headers.origin} subscribed at ${socket.handshake.time}`
  );
});
// API Endpoint to decompress incomming gz file and to send content back.
app.post("/decompress-gzips", uploadsConfig.single("uploaded_file"), async (req, res) => {
  let newFileName = req.file.filename.replace(".gz", "");
  let file = req.file;
  if (!file) {
    const error = new Error("Please upload a file");
    error.httpStatusCode = 400;
    return next(error);
  }
  const input = fs.createReadStream(file.path);

  const output = fs.createWriteStream(path.join(DIR, newFileName));
  let unzippedFile = input.pipe(zlib.createUnzip());
  let writeNewFile = unzippedFile.pipe(output);
  writeNewFile.on("finish", () => {
    res.header("Content-Type", "application/json");
    res.sendFile(path.join(__dirname, DIR, newFileName));
  });
});

// API Endpoint to delete files to save storage.
app.delete("/delete-gzips/:filename", async (req, res) => {
  let filename = req.params.filename;
  fs.unlinkSync(path.join(__dirname, DIR, filename));
  fs.unlinkSync(path.join(__dirname, DIR, filename.replace(".gz", "")));

  return res.status(200);
});

// API Endpoint to trigger a replay simulation remotely.
app.get("/remote/start-replay", async (req, res) => {
  let clients = Array.from(await io.allSockets());
  let lastClient = clients[clients.length - 1];
  // Before sending data the client needs to setup correctly.
  io.to(lastClient).emit("set_replay_environment", {});
  await getReplayData(req.query.id, req.query.name);
  getPlotData(req.query.id);
});

// Search and send replay data to client.
async function getReplayData(dir, name) {
  let clients = Array.from(await io.allSockets());
  let lastClient = clients[clients.length - 1];
  let filePath = path.join(FILEPATHROOT, FILEDIRECTORY, dir, name);
  let fileNameParts = name.split(".");
  let fileType = fileNameParts[fileNameParts.length - 1];

  if (fileType === "json") {
    fs.createReadStream(filePath)
      .pipe(JSONStream.parse("*"))
      .on("data", function(data) {
        //send Data via websocket
        io.to(lastClient).emit("new_replay_data", data);
      })
      .on("end", function() {
        io.to(lastClient).emit("replay_data_sent", {});
        console.log("last Data sent");
      });
  } else {
    fs.createReadStream(filePath)
      .pipe(zlib.createUnzip())
      .pipe(JSONStream.parse("*"))
      .on("data", function(data) {
        //send Data via websocket
        io.to(lastClient).emit("new_replay_data", data);
      })
      .on("end", function() {
        io.to(lastClient).emit("replay_data_sent", {});
        console.log("last Data sent");
      });
  }
}

// Search and send plot file to client.
async function getPlotData(query) {
  let clients = Array.from(await io.allSockets());
  let lastClient = clients[clients.length - 1];
  let filePath = path.join(FILEPATHROOT, FILEDIRECTORY, query, query + ".html");
  fs.readFile(filePath, "utf8", function(err, data) {
    if (err) {
      console.log(`File not found: ${err}`)
    }else {
      io.to(lastClient).emit("plot_data", data);
    }
  });
}
