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
var DIR = "./uploads";

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

// Confirm that server is running and is listening to the specified port.
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
// API Endpoint to decompress incomming gz file and sent content back.
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
  getReplayData(req.query.id, req.query.name);
  setTimeout(() => {
    getPlotData(req.query.id);
  }, 1000);
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
    io.to(lastClient).emit("plot_data", data);
  });
}
