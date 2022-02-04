# Cloe UI Webserver

The Cloe UI Webserver provides two main features:

- replay a recorded .JSON or .JSON.gz file
- unzip the JSON.gz files

## Getting Started

The fastest way to get started is to build and run a Docker image. In
`ui/server`, do:

```
    make image run
```

In case the build gets stuck, try increasing the ulimit.

## Usage

### Replaying Recorded Simulation Data

#### Prerequisite

To be able to replay simulation data, you have to first run a simulation with
the `api_recording` config in the stackfile. This config creates a .JSON file
which is necessary for the replay.

Example stackfile snippet:

```json
{
  "engine": {
    "output": {
      "files": {
        "api_recording": "<MY_OUTPUT_DIR>/data.json.gz"
      }
    }
  }
}
```

### Main usage

The `ui/scripts/launch_replay.py` script is provided to visualize recorded data
with the cloe-ui. It can be used like so:

```
  ui/scripts/launch_replay.py --path <PATH TO data.json or data.json.gz>
                              --cloe-ui-image cloe/cloe-ui:<UI TAG>
                              --webserver-image cloe/cloe-ui-webserver:<SERVER TAG>
```

For details on the usage, do

```
  ui/scripts/launch_replay.py --help
```

The script will build and run the cloe-ui and cloe-ui-webserver docker images
and automatically visualize the recorded data in the browser.

The cloe-ui is accessible via port 5000 if running in a Docker container. In
case of a local build, port 3000 is used.
