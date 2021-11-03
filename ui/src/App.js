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
import React, { Component } from "react";
import { withCookies } from "react-cookie";
import axios from "axios";
import { DragDropContext, Draggable } from "react-beautiful-dnd";
import { Spin, Progress } from "antd";
import RecordService from "./services/recordService";
import Sidebar from "./components/sidebar";
import ErrorBoundary from "./components/errorBoundary";
import NavBar from "./components/navbar";
import SimulatorBinding from "./components/simulatorBinding";
import Simulation from "./components/simulation";
import Triggers from "./components/triggers";
import Rendering from "./components/rendering";
import Controller from "./components/controller";
import DroppableWrapper from "./components/droppableWrapper";
import VehicleCard from "./components/vehiclecard";
import { REPLAYSTATES, STREAMINGTYPES } from "./enums";
import ErrorMessage from "./components/errorMessage";
import PlotData from "./components/plotData";
import socketIOClient from "socket.io-client";
import { download } from "../src/helpers";

class App extends Component {
  constructor(props) {
    super(props);

    // This prefix will be added to each endpoint during the fetch process.
    this.apiPrefix = "/api";

    // Definition of the endpoints to fetch, specific to startup phase.
    this.firstPhaseEndpoints = [""];
    this.secondPhaseEndpoints = ["/uuid", "/version", "/progress"];
    this.thirdPhaseEndpoints = this.setDefaultEndpoints();
    this.replayPhaseEndpoints = this.setDefaultEndpoints();

    // this.endpointsToFetch gets filled with the third phase endpoints and
    // dynamic named endpoints, e.g. for controllers or simulators.
    this.endpointsToFetch = [];
    this.controllerEndpoints = [];

    this.replayBufferData = [];
    // All data which could change during one simulation is stored in this.state.
    this.state = {
      startupPhase: 1,
      cloeDataImported: {},
      cloeDataLive: {},
      configData: {},
      initialHost: "",
      host: "",
      updateInterval: 500,
      connected: false,
      sideBarOpen: false,
      dragNDropActivated: false,
      streamingType: STREAMINGTYPES.LIVE,
      replayIndex: 0,
      replayState: REPLAYSTATES.PAUSED,
      showErrorMessage: false,
      isRecoding: false,
      isBuffering: false
    };

    // Helper variable to declare if Cloe-UI should perform one-time fetches,
    // this will be the case at each start of a new simulation.
    this.performOneTimeFetches = true;
    this.jsonVersionChecked = false;
    this.fetchCloeApi = 0;
    this.bufferInterval = 0;
    this.defaultUpdateInterval = 500;
    // Definitions for the one-time fetched information.
    this.controllers = [];
    this.simulators = [];
    this.vehicles = [];
    this.triggerActions = [];
    this.triggerEvents = [];

    // Definitions for startup phase 1 and 2 variables.
    this.uuid = null;
    this.version = null;
    this.initializationProgress = 0;

    // Definition of which component should be displayed at which app column
    // per default, if no cookie with stored layout is available.
    const { cookies } = props;
    this.componentsOnLeftSide = cookies.get("firstColumn") || [0, 6];
    this.componentsOnRightSide = cookies.get("secondColumn") || [1, 2, 3, 4, 5];
    this.componentOrder = ["left"];
    this.recordService = null;

    this.webserverPort = 4000;
    this.stepWidth = null;
    this.replaySpeed = 1;
    // URL Parameter to define where to ask for data.
    let hostParameter = this.getQueryVariable("host")
      ? this.getQueryVariable("host")
      : "localhost:8080";
    let url = new URL("http://" + hostParameter);
    this.hostname = url.hostname;
    this.port = url.port;
    this.host = url.host;

    // URL Parameter to trigger a replay and to define which tc should be replayed.
    this.remoteReplayId = this.getQueryVariable("id") ? this.getQueryVariable("id") : "";
    // URL Parameter to define the name of the replay file.
    this.fileName = this.getQueryVariable("name") ? this.getQueryVariable("name") : "";
    // Connect to host via websockets to transfer data for replay.
    this.socket = socketIOClient(`ws://${this.hostname}:${this.webserverPort}`);
    this.child = React.createRef();
  }

  render() {
    // Perform one-time fetches if new simulation started.
    if (this.state.startupPhase === 3 && this.performOneTimeFetches) {
      this.getControllerEndpoints();
      this.getSimulatorEndpoints();
      this.getVehicleEndpoints();
      this.getActionsAndEvents();
      this.performOneTimeFetches = false;
    }
    const {
      connected,
      updateInterval,
      host,
      initialHost,
      sideBarOpen,
      cloeDataLive,
      configData
    } = this.state;
    // The following array includes all main components.
    const components = [
      {
        id: 0,
        component: this.controllers
          ? this.controllers.map((singleController) => (
              <Controller
                key={singleController.endpointBase}
                host={host}
                apiPrefix={this.apiPrefix}
                controller={singleController}
                connected={connected}
                updateInterval={updateInterval}
                cloeData={cloeDataLive}
                simulationID={this.uuid}
                uiConfigData={
                  configData[`${this.apiPrefix}/controllers/${singleController.id}/ui`] || {}
                }
                streamingType={this.state.streamingType}
                saveLayout={(name, value) => this.cookiesSet(name, value)}
                layout={this.cookiesGet("controllerLayout") || {}}
              />
            ))
          : ""
      },
      {
        id: 1,
        component: (
          <Rendering
            sensors={cloeDataLive.sensors}
            replayState={this.state.replayState}
            isBuffering={this.state.isBuffering}
            streamingType={this.state.streamingType}
            isRecording={this.state.isRecoding}
            toggleReplay={this.toggleReplay}
            fastForward={this.fastForward}
            rewind={this.rewind}
            recordCanvas={this.recordCanvas}
          />
        )
      },
      {
        id: 2,
        component: (
          <Simulation
            simulation={this.getSimulationState(cloeDataLive["/simulation"])}
            host={host}
            apiPrefix={this.apiPrefix}
            uuid={this.uuid}
            setSimulationSpeed={this.setSimulationSpeed}
            streamingType={this.state.streamingType}
          />
        )
      },
      {
        id: 3,
        component:
          this.simulators.length === 0 ? (
            <SimulatorBinding simulator={null} />
          ) : (
            this.simulators.map((simulator) => (
              <SimulatorBinding simulator={this.state.cloeDataLive[simulator]} key={simulator} />
            ))
          )
      },
      {
        id: 4,
        component:
          this.vehicles.length === 0 ? (
            <VehicleCard vehicle={null} />
          ) : (
            this.vehicles.map((vehicle) => (
              <VehicleCard
                vehicle={this.state.cloeDataLive[vehicle]}
                key={vehicle}
                simTime={cloeDataLive.simTime}
                simulationID={this.uuid}
              />
            ))
          )
      },
      {
        id: 5,
        component: (
          <Triggers
            host={host}
            apiPrefix={this.apiPrefix}
            connected={connected}
            triggers={{
              queue: cloeDataLive["/triggers/queue"],
              history: cloeDataLive["/triggers/history"]
            }}
            triggerEvents={this.triggerEvents}
            triggerActions={this.triggerActions}
          />
        )
      },
      {
        id: 6,
        component: (
          <PlotData
            ref={this.child}
            simTime={cloeDataLive.simTime}
            replayState={this.state.replayState}
          />
        )
      }
    ];

    // Render components.
    return (
      <div className="App" ref={(ref) => (this.el = ref)}>
        <ErrorBoundary>
          <NavBar
            connected={connected}
            toggleSidebar={this.toggleSidebar}
            getSimulationDataFromJSON={this.getSimulationDataFromJSON}
            setupReplayEnvironment={this.setupReplayEnvironment}
            version={this.version ? this.version : ""}
            hostname={this.hostname}
            webserverPort={this.webserverPort}
          />
        </ErrorBoundary>
        <div className="container pt-4">
          <Sidebar
            host={host}
            initialHost={initialHost}
            apiPrefix={this.apiPrefix}
            sideBarOpen={sideBarOpen}
            closeSidebar={() => this.setState({ sideBarOpen: false })}
            connected={connected}
            updateInterval={updateInterval}
            updateHost={this.updateHost}
            handleEnter={this.handleEnter}
            draggingActivated={this.state.dragNDropActivated}
            handleInterval={(value) => {
              this.setState({ updateInterval: value });
            }}
            setUpdateInterval={this.setUpdateInterval}
            handleDragNDropSwitch={() =>
              this.setState({
                dragNDropActivated: !this.state.dragNDropActivated
              })
            }
            resetCookies={this.resetCookies}
          />
          {this.state.showErrorMessage && (
            <ErrorMessage
              errorMessage={`current JSON ${this.version || ""} version is not supported`}
            ></ErrorMessage>
          )}
          {this.renderComponents(components)}
        </div>
      </div>
    );
  }

  renderComponents = (components) => {
    // Depending on the current startup phase, render different components.
    // Startup Phase 1: Render a message that UI is waiting for a Cloe host.
    // Startup Phase 2: Render an indicator for the Cloe initialization progress.
    // Startup Phase 3: Cloe is fully initialized, so render all UI components.
    // Startup Phase 4: Cloe is stopped, but last simulation state should still be rendered.
    const waitingForCloeHost = <Spin className="mr-2" tip="Waiting for Cloe Host..." />;
    const waitingForCloeInit = (
      <React.Fragment>
        <div className="m-auto p-2" style={{ width: "100px" }}>
          <Progress
            type="circle"
            percent={Math.round((this.initializationProgress || 0) * 100)}
            width={80}
          />
        </div>
        <div style={{ color: "#005691", fontSize: "14px" }}>Waiting for Cloe Initialisation...</div>
      </React.Fragment>
    );
    // The items of the components argument will be distributed to the
    // two app columns.
    //
    // Which component will be on which app column is defined
    // in this.componentsOnLeftSide and this.componentsOnRightSide.
    const cloeUiComponents = (
      <DragDropContext
        onBeforeDragStart={this.onBeforeDragStart}
        onDragStart={this.onDragStart}
        onDragUpdate={this.onDragUpdate}
        onDragEnd={this.onDragEnd}
      >
        <div className="row">
          {this.addDroppableWrapper(1, this.componentsOnLeftSide, components)}
          {this.addDroppableWrapper(2, this.componentsOnRightSide, components)}
        </div>
      </DragDropContext>
    );
    switch (this.state.startupPhase) {
      case 1:
        return waitingForCloeHost;
      case 2:
        return waitingForCloeInit;
      case 3:
        return cloeUiComponents;
      case 4:
        return cloeUiComponents;
      default:
        return null;
    }
  };

  addDroppableWrapper = (id, column, components) => {
    return (
      <DroppableWrapper droppableId={id} moveComponent={this.moveComponent}>
        {column.map((itemID, index) => (
          <Draggable
            draggableId={itemID.toString()}
            key={itemID.toString()}
            index={index}
            isDragDisabled={!this.state.dragNDropActivated}
          >
            {(provided, snapshot) => (
              <div
                ref={provided.innerRef}
                {...provided.draggableProps}
                {...provided.dragHandleProps}
                style={{
                  ...provided.draggableProps.style,
                  margin: "0 0 15px 0"
                }}
              >
                <ErrorBoundary>
                  {components.find((item) => item.id === itemID).component}
                </ErrorBoundary>
              </div>
            )}
          </Draggable>
        ))}
      </DroppableWrapper>
    );
  };

  componentDidMount() {
    // Set host variable with get parameter or default value and
    // store the initial host in order to have it as placeholder in input field
    // of the sidebar component.

    this.setState({
      initialHost: this.host,
      host: this.host
    });

    // Fetch data from Cloe API in given interval (default: 500ms).
    this.fetchCloeApi = setInterval(this.fetchData, this.defaultUpdateInterval);

    this.websocketCallback();
  }

  // Remove the listener before unmounting the component to avoid addition of multiple listeners.
  componentWillUnmount() {
    this.socket.off("new_replay_data");
    this.socket.off("replay_data_sent");
    this.socket.off("plot_data");
  }

  websocketCallback = () => {
    this.socket.on("set_replay_environment", async (data) => {
      console.log("new simulation replay started");
      this.replayBufferData = [];
      this.setState({ isBuffering: true });
      this.setupReplayEnvironment();
    });

    this.socket.on("replay_data_sent", async (data) => {
      console.log("data sent");
      this.stopBuffering();
      let dataArray = this.replayBufferData.flat();
      dataArray.shift();
      this.setState({ cloeDataImported: dataArray, isBuffering: false });
    });

    this.socket.on("new_replay_data", async (dataChunks) => {
      let parsedData = this.setEndpointsFromArray([dataChunks]);
      this.replayBufferData.push(parsedData);
      if (!this.jsonVersionChecked) {
        this.checkJSONVersion(this.replayBufferData.flat());
        this.startBuffering();
        setTimeout(() => {
          this.renewFetchInterval(this.state.updateInterval);
        }, 1000);
      }
    });

    this.socket.on("plot_data", (data) => {
      if (data != null) {
        this.loadPlotData(data);
        this.setState({ isBuffering: false });
      }
    });

    this.socket.on("connect", () => {
      if (this.remoteReplayId) {
        fetch(
          `http://${this.hostname}:${this.webserverPort}/remote/start-replay?id=${this.remoteReplayId}&name=${this.fileName}`
        );
      }
    });

    this.socket.on("disconnect", () => {});
  };

  fetchData = () => {
    // This function checks for connection, and, depending on
    // the startup phase, fetches a set of endpoints.
    if (!this.state.streamingType === STREAMINGTYPES.LIVE) {
      return;
    }
    // Check always for connection.
    this.setConnectionState();
    // In StartupPhase 2, fetch UUID, version and Cloe init progress.
    if (this.state.startupPhase === 2) {
      this.startupStepTwo();
    }

    // In StartupPhase 3, fetch all endpoints in this.endpoints, these
    // include all relevant Cloe data. The data will be stored in
    // this.state.cloeDataLive.
    if (this.state.startupPhase === 3) {
      this.startupStepThree();
    }

    if (this.state.startupPhase === 4) {
      this.startupStepFour();
    }
  };

  // This method is used to rearange components between the two app columns.
  moveComponent = (componentId, sourceColumnId, targetColumnId, targetIndex) => {
    if (sourceColumnId === targetColumnId) {
      switch (targetColumnId) {
        case 1:
          this.componentsOnLeftSide.splice(this.componentsOnLeftSide.indexOf(componentId), 1);
          this.componentsOnLeftSide.splice(targetIndex, 0, componentId);
          break;
        case 2:
          this.componentsOnRightSide.splice(this.componentsOnRightSide.indexOf(componentId), 1);
          this.componentsOnRightSide.splice(targetIndex, 0, componentId);
          break;
        default:
          break;
      }
    } else {
      switch (targetColumnId) {
        case 1:
          this.componentsOnRightSide.splice(this.componentsOnRightSide.indexOf(componentId), 1);
          this.componentsOnLeftSide.splice(targetIndex, 0, componentId);
          break;
        case 2:
          this.componentsOnLeftSide.splice(this.componentsOnLeftSide.indexOf(componentId), 1);
          this.componentsOnRightSide.splice(targetIndex, 0, componentId);
          break;
        default:
          break;
      }
    }
    this.forceUpdate();
    const { cookies } = this.props;
    cookies.set("firstColumn", this.componentsOnLeftSide, { path: "/" });
    cookies.set("secondColumn", this.componentsOnRightSide, { path: "/" });
  };

  setConnectionState = () => {
    axios
      .get(`http://${this.state.host}`)
      .then(() => {
        // If connected set new interval.
        this.renewFetchInterval(this.defaultUpdateInterval);
        if (!this.state.connected) {
          this.setState({ connected: true, startupPhase: 2 });
        }
      })
      .catch(() => {
        this.setState({ connected: false });
        if (this.startupPhase === 2) {
          this.setState({ startupPhase: 1 });
        }
        if (this.startupPhase === 3) {
          this.setState({ startupPhase: 4 });
        }
      });
  };

  startupStepTwo = () => {
    axios
      // Get the data from each endpoint.
      .all(this.secondPhaseEndpoints.map((endpoint) => this.getData(this.state.host, endpoint)))
      .then(
        axios.spread(
          function(...allResults) {
            // Create temporary data object which holds all data.
            if (allResults[0] === undefined) {
              this.setState({ startupPhase: 1 });
              this.renewFetchInterval(this.state.updateInterval);
              return;
            }
            var data = {};
            for (const index in allResults) {
              data[this.secondPhaseEndpoints[index]] = (allResults[index] || {}).data;
            }
            this.version = data["/version"];
            this.initializationProgress = data["/progress"].initialization.percent;
            if (this.initializationProgress === 1) {
              this.setState({ startupPhase: 3 });
              if (this.uuid !== data["/uuid"]) {
                // New simulation started; make sure that dynamic endpoints
                // get fetched.
                this.uuid = data["/uuid"];
                this.performOneTimeFetches = true;
                this.endpointsToFetch = this.thirdPhaseEndpoints.slice();
                this.renewFetchInterval(this.state.updateInterval);
              }
            }
            // Clear data to ensure that at a new simulation, Cloe-UI doesn't
            // show old information.
            this.setState({ cloeDataLive: {} });
          }.bind(this)
        )
      );
  };

  startupStepThree = () => {
    if (!this.state.connected) {
      this.setState({ startupPhase: 4 });
    } else {
      axios
        // Get the data from each endpoint.
        .all(this.endpointsToFetch.map((endpoint) => this.getData(this.state.host, endpoint)))
        .then(
          axios.spread(
            function(...allResults) {
              // Create temporary data object which holds all data.
              var data = {};
              for (const index in allResults) {
                data[this.endpointsToFetch[index]] = (allResults[index] || {}).data;
              }
              // Store simTime and sensors in a more accessible way.
              data.simTime = ((data["/simulation"] || {}).time || {}).ms;
              data.sensors = (data[this.vehicles[0]] || {}).components;
              // Update app state which triggers rerendering of the app.
              if (this.state.connected) {
                this.setState({ cloeDataLive: data });
              }
            }.bind(this)
          )
        );
    }
  };

  startupStepFour = () => {
    if (this.state.connected) {
      this.setState({ startupPhase: 2 });
      this.renewFetchInterval(100);
    }
  };

  // This method is used to fetch and return data from a specified host and endpoint.
  getData = (host, endpoint) => {
    return axios.get(`http://${host}${this.apiPrefix}${endpoint}`).catch(function(error) {
      if (error.response) {
        console.log(error);
      }
    });
  };

  // This method helps to determine if a Cloe host is defined
  // as get parameter and returns it if true.
  getQueryVariable = (variable) => {
    var query = window.location.search.substring(1);
    var vars = query.split("&");
    for (let i = 0; i < vars.length; i++) {
      var pair = vars[i].split("=");
      if (pair[0] === variable) {
        return pair[1];
      }
    }
    return false;
  };

  // This method will create an array of controller objects with
  // an id and the base endpoint path (like /cloeApi/controllers/1).
  getControllerEndpoints = () => {
    this.getData(this.state.host, "/controllers").then((data) => {
      this.controllers = [];
      for (const index in data.data) {
        const controllerID = data.data[index];
        const controller = {
          id: controllerID,
          endpointBase: `${this.apiPrefix}/controllers/${controllerID}`
        };
        this.controllers.push(controller);
      }
    });
  };

  // This method will fill an array of simulator binding objects with
  // an id and the base endpoint path (like /cloeApi/simulator/vtd/state).
  getSimulatorEndpoints = () => {
    this.getData(this.state.host, "/simulators").then((data) => {
      this.simulators = [];
      for (const index in data.data) {
        const simulatorID = data.data[index];
        this.endpointsToFetch.push(`/simulators/${simulatorID}/state`);
        this.simulators.push(`/simulators/${simulatorID}/state`);
      }
    });
  };

  // This method will fill an array of vehicle objects with
  // an id and the base endpoint path (like /cloeApi/vehicles/default).
  getVehicleEndpoints = () => {
    this.getData(this.state.host, "/vehicles").then((data) => {
      this.vehicles = [];
      for (const index in data.data) {
        const vehicleID = data.data[index];
        this.endpointsToFetch.push(`/vehicles/${vehicleID}`);
        this.vehicles.push(`/vehicles/${vehicleID}`);
      }
    });
  };

  // This method will create two arrays with the available actions and events
  // for the simulation.
  getActionsAndEvents = () => {
    this.getData(this.state.host, "/triggers/actions").then((data) => {
      this.triggerActions = data.data;
    });
    this.getData(this.state.host, "/triggers/events").then((data) => {
      this.triggerEvents = data.data;
    });
  };

  resetCookies = () => {
    const { cookies } = this.props;
    for (const cookie in cookies.getAll()) {
      cookies.remove(cookie);
    }
    this.componentsOnLeftSide = [0, 6];
    this.componentsOnRightSide = [1, 2, 3, 4, 5];
  };

  updateHost = () => {
    clearInterval(this.fetchCloeApi);
    this.thirdPhaseEndpoints = this.setDefaultEndpoints();
    if (document.getElementById("cloeHost").value === "") {
      this.setState({ host: this.state.initialHost });
    } else {
      this.setState({ host: document.getElementById("cloeHost").value });
    }
    this.setState(
      {
        streamingType: STREAMINGTYPES.LIVE,
        startupPhase: 1,
        updateInterval: this.defaultUpdateInterval
      },
      () => {
        this.initializationProgress = 0;
        this.renewFetchInterval(this.defaultUpdateInterval);
      }
    );
  };

  handleEnter = (e) => {
    if (e.keyCode === 13) {
      this.updateHost();
    }
  };

  toggleSidebar = () => {
    this.setState({ sideBarOpen: !this.state.sideBarOpen });
  };

  cookiesSet(name, value) {
    const { cookies } = this.props;
    cookies.set(name, value, { path: "/" });
  }

  cookiesGet(name) {
    const { cookies } = this.props;
    return cookies.get(name);
  }

  setUpdateInterval = (value) => {
    this.setState({ updateInterval: value }, this.renewFetchInterval(value));
  };

  renewFetchInterval = (updateInterval) => {
    if (this.fetchCloeApi) {
      clearInterval(this.fetchCloeApi);
    }
    if (this.state.streamingType === STREAMINGTYPES.LIVE) {
      this.fetchData();
      this.fetchCloeApi = setInterval(this.fetchData, updateInterval);
    } else {
      this.setSimDataBuffer(this.state.cloeDataImported[0]);
    }
  };

  getSimulationState = (simulationState) => {
    if (simulationState) {
      return {
        time: simulationState.time.ms,
        step: simulationState.step,
        realtime_factor: simulationState.realtime_factor,
        eta: simulationState.eta,
        stepWidth: parseInt(simulationState.step_width)
      };
    } else {
      return undefined;
    }
  };

  onDragEnd = (result) => {
    const { destination, source, draggableId } = result;
    if (!destination) {
      return;
    }
    if (destination.droppableId === source.droppableId && destination.index === source.index) {
      return;
    }
    this.moveComponent(
      parseInt(draggableId),
      source.droppableId,
      destination.droppableId,
      destination.index
    );
  };

  setSimulationSpeed = (value) => {
    this.replaySpeed = Number(((this.state.updateInterval * value) / this.stepWidth).toFixed());
    this.startReplay(this.replaySpeed);
  };

  resetEndpoints = () => {
    this.controllerEndpoints = [];
    this.vehicles = [];
    this.simulators = [];
    this.controllers = [];
    this.triggerActions = [];
    this.triggerEvents = [];
  };

  setDefaultEndpoints = () => {
    return ["/simulation", "/triggers/queue", "/triggers/history", "/uuid", "/version"];
  };

  // The following functions are only needed for replay.

  recordCanvas = () => {
    let canvas = document.getElementsByTagName("canvas")[1];
    this.setState({ isRecoding: !this.state.isRecoding }, () => {
      const recordSettings = {
        timeslice: 100,
        mimeType: "video/webm",
        canvas: canvas
      };
      if (this.recordService === null) {
        this.recordService = new RecordService(recordSettings);
      }
      if (this.state.isRecoding) {
        this.recordService.startRecording();
      } else {
        this.recordService.stopRecording();
        let videoData = this.recordService.getData();
        videoData.then((data) => {
          const url = URL.createObjectURL(data);
          download(url, "cloe-replay-" + this.uuid);
          this.recordService = null;
        });
      }
    });
  };

  rewind = () => {
    let currentIndex = this.state.replayIndex;
    this.setState({ replayState: REPLAYSTATES.PAUSED }, () => {
      currentIndex =
        currentIndex - 100 <= this.state.replayPauseIndex
          ? this.state.replayPauseIndex
          : (currentIndex -= 100);
      this.setState({ replayIndex: currentIndex }, () => {
        clearInterval(this.fetchCloeApi);
        this.startReplay(this.replaySpeed);
        this.setState({ replayState: REPLAYSTATES.STARTED });
      });
    });
  };

  fastForward = () => {
    let currentIndex = this.state.replayIndex;
    this.setState({ replayState: REPLAYSTATES.PAUSED }, () => {
      currentIndex =
        currentIndex + 100 >= this.state.cloeDataImported.length
          ? (currentIndex = this.state.cloeDataImported.length)
          : (currentIndex += 100);
      this.setState({ replayIndex: currentIndex }, () => {
        clearInterval(this.fetchCloeApi);
        this.startReplay(this.replaySpeed);
        this.setState({ replayState: REPLAYSTATES.STARTED });
      });
    });
  };

  toggleReplay = (replayState) => {
    if (replayState === REPLAYSTATES.FINISHED) {
      this.setState({ replayState: REPLAYSTATES.STARTED, replayIndex: 0 }, () => {
        clearInterval(this.fetchCloeApi);
        this.startReplay(this.replaySpeed);
      });
    } else if (replayState === REPLAYSTATES.STARTED) {
      this.setState({ replayState: replayState }, () => {
        clearInterval(this.fetchCloeApi);
        this.stepWidth = this.getSimulationState(
          this.state.cloeDataImported[0][this.apiPrefix + "/simulation"]
        ).stepWidth;
        this.replaySpeed = this.state.updateInterval / this.stepWidth;
        this.startReplay(this.replaySpeed);
      });
    } else {
      clearInterval(this.fetchCloeApi);
      this.setState({ replayState: REPLAYSTATES.PAUSED });
    }
  };

  startReplay = (replaySpeed) => {
    clearInterval(this.fetchCloeApi);
    this.fetchCloeApi = setInterval(() => {
      let index = this.state.replayIndex;
      if (
        index < this.state.cloeDataImported.length &&
        this.state.replayState === REPLAYSTATES.STARTED
      ) {
        this.setSimDataBuffer(this.state.cloeDataImported[index]);
        index += replaySpeed;
        this.setState({ replayIndex: index });
      } else {
        this.setState({ replayState: REPLAYSTATES.FINISHED });
        clearInterval(this.fetchCloeApi);
        return;
      }
    }, this.state.updateInterval);
  };

  setSimDataBuffer = (allResults) => {
    if (allResults === undefined) return;
    // Create temporary data object which holds all data.
    let data = {};
    if (this.state.startupPhase === 2) {
      this.version = allResults[this.apiPrefix + "/version"];
      this.uuid = allResults[this.apiPrefix + "/uuid"];
      this.initializationProgress = 1;
      this.controllers = [];
      for (const index in allResults[this.apiPrefix + "/controllers"]) {
        const controllerID = allResults[this.apiPrefix + "/controllers"][index];
        const controller = {
          id: controllerID,
          endpointBase: `${this.apiPrefix}/controllers/${controllerID}`
        };
        this.controllers.push(controller);
      }
      this.triggerEvents = allResults[this.apiPrefix + "/triggers/events"];
      this.triggerActions = allResults[this.apiPrefix + "/triggers/actions"];
      this.setState({ startupPhase: 3, configData: allResults });
    } else if (this.state.startupPhase === 3) {
      for (let index = 0; index < this.replayPhaseEndpoints.length; index++) {
        data[this.replayPhaseEndpoints[index]] =
          allResults[this.apiPrefix + this.replayPhaseEndpoints[index]] || {};
      }
      // Store simTime and sensors in a more accessible way.
      data.simTime = ((data["/simulation"] || {}).time || {}).ms;
      data.sensors = (data[this.vehicles[0]] || {}).components;
      // Update app state which triggers rerendering of the app.
      this.setState({ cloeDataLive: data });
    }
  };

  getSimulationDataFromJSON = (file, content) => {
    this.resetEndpoints();
    let parsedData = this.setEndpointsFromArray(content);
    if (!this.jsonVersionChecked) {
      this.checkJSONVersion(parsedData);
    }
  };

  setupReplayEnvironment = () => {
    this.resetEndpoints();
    this.initializationProgress = 0.5;
    clearInterval(this.fetchCloeApi);
    this.setState({
      streamingType: STREAMINGTYPES.REPLAY,
      replayIndex: 0,
      updateInterval: 100,
      startupPhase: 2,
      cloeData: {}
    });
    this.performOneTimeFetches = false;
  };

  startBuffering = () => {
    console.log("start Buffering");
    this.bufferInterval = setInterval(() => {
      let dataArray = this.replayBufferData.flat();
      dataArray.shift();
      this.setState({ cloeDataImported: dataArray });
    }, this.defaultUpdateInterval);
  };

  stopBuffering = () => {
    console.log("stop Buffering");
    clearInterval(this.bufferInterval);
  };

  loadPlotData = (data) => {
    if (this.child.current == null) {
      setTimeout(() => {
        this.loadPlotData(data);
      }, 500);
    } else {
      console.log("wad");
      console.log(this.child.current);
      this.child.current.extractDataFromString(data);
    }
  };

  checkJSONVersion = (cloeDataImported) => {
    // Get first element which contains meta infos.
    let configData = cloeDataImported.shift();
    // Check if JSON is valid and currently supported.
    let destructVersion = configData[this.apiPrefix + "/version"].split(".");
    let version = destructVersion[0] + "." + destructVersion[1] + destructVersion[2];
    if (parseFloat(version) >= 0.18) {
      this.triggerActions = configData[this.apiPrefix + "/triggers/actions"];
      this.triggerEvents = configData[this.apiPrefix + "/triggers/events"];
      this.setState({ cloeDataImported: cloeDataImported }, () =>
        this.setSimDataBuffer(configData)
      );
    } else {
      this.setState({ showErrorMessage: true });
      setTimeout(() => {
        this.setState({ showErrorMessage: false });
      }, 5000);
    }
    this.jsonVersionChecked = true;
  };

  setEndpointsFromArray = (content) => {
    const simulationData = typeof content === "string" ? JSON.parse(content) : content;
    // Parse data to valid JSON.
    let parsedData = simulationData.map((arr, i) => {
      var data = {};
      for (const key in arr) {
        data[key] = JSON.parse(arr[key]);
        // Set simulator endpoint from given JSON.
        let endpoint = key.replace(this.apiPrefix, "");
        if (key.match(/simulators.*state$/) && !this.simulators.includes(endpoint)) {
          this.simulators.push(endpoint);
          this.replayPhaseEndpoints.push(endpoint);
        }
        if (key.match(/vehicles./) && !this.vehicles.includes(endpoint)) {
          this.vehicles.push(endpoint);
          this.replayPhaseEndpoints.push(endpoint);
        }
        if (key.match(/controllers./) && !this.controllerEndpoints.includes(endpoint)) {
          this.controllerEndpoints.push(endpoint);
          this.replayPhaseEndpoints.push(endpoint);
        }
      }
      return data;
    });
    return parsedData;
  };
}

export default withCookies(App);
