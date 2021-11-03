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
import Card from "./card";
import ToggleSwitch from "./toggleSwitch";
import * as THREE from "three";
import Slider from "react-rangeslider";
import { Select, Tag } from "antd";
import { SettingFilled, UndoOutlined } from "@ant-design/icons";
import SpriteText from "three-spritetext";
import ReplayControlls from "./replayControls";

const OrbitControls = require("three-orbit-controls")(THREE);
var randomColor = require("randomcolor");
const Option = Select.Option;

class Rendering extends Component {
  constructor(props) {
    super(props);
    this.start = this.start.bind(this);
    this.stop = this.stop.bind(this);
    this.animate = this.animate.bind(this);
    this.vehicleMaterial = new THREE.MeshLambertMaterial({
      color: 0x0d47a1,
      transparent: true,
      opacity: 0.5
    });
    this.egoVehicleMaterial = new THREE.MeshLambertMaterial({
      color: 0x129a7d,
      transparent: true,
      opacity: 0.3
    });
    this.sensorMaterial = new THREE.MeshLambertMaterial({
      vertexColors: true,
      color: 0xfffb8f,
      transparent: true,
      opacity: 0.25
    });
    this.wfMaterial = new THREE.LineBasicMaterial({
      color: 0xaaaaaa,
      linewidth: 1
    });
    this.cameraPerspecives = [
      {
        camera: { x: -10, y: -5, z: 5 },
        target: { x: 0, y: 0, z: 0 },
        name: "Default"
      },
      {
        camera: { x: -6, y: 0, z: 3 },
        target: { x: 0, y: 0, z: 2 },
        name: "Third Person"
      },
      {
        camera: { x: 3, y: 0, z: 1 },
        target: { x: 8, y: 0, z: 1 },
        name: "Ego"
      },
      {
        camera: { x: 12, y: 0, z: 45 },
        target: { x: 12 + 1e-6, y: 0, z: 1 },
        name: "Bird"
      }
    ];
    this.defaultCameraPerspective = 0;
    this.currentCameraPerspective = this.defaultCameraPerspective;
    this.defaultColors = {
      Object: ["#999999", "#8ac0de", "#e6b655", "#e58b88", "#70ae98"],
      Lane: ["#e6b655", "#e58b88", "#8ac0de", "#70ae98"]
    };
    this.sensorType = { Object: "object", Lane: "lane" };
    this.defaultSensor = {
      Object: "cloe::default_world_sensor",
      Lane: "cloe::default_lane_sensor",
      Ego: "cloe::default_ego_sensor"
    };
    this.objColors = {};
    this.objColors[this.defaultSensor.Object] = this.defaultColors.Object[0];
    this.laneColors = {};
    this.laneColors[this.defaultSensor.Lane] = this.defaultColors.Lane[0];
    this.state = {
      optionsVisible: false,
      helperAxesVisible: false,
      gridVisible: true,
      gridCellsize: 1,
      currentPerspective: "Default",
      frustum3d: false,
      renderLabels: false,
      objSensorsToRender: [this.defaultSensor.Object],
      laneSensorsToRender: [this.defaultSensor.Lane]
    };
  }

  render() {
    // Make sensors available for input fields.
    const { sensors, replayState, streamingType, isRecording, isBuffering } = this.props;
    const selectObjSensors = this.getAvailSensors(sensors, this.sensorType.Object);
    const selectLaneSensors = this.getAvailSensors(sensors, this.sensorType.Lane);

    if (this._isMounted) {
      this.updateSensedEnvironment();
    }
    const {
      optionsVisible,
      helperAxesVisible,
      gridVisible,
      gridCellsize,
      currentPerspective,
      frustum3d,
      renderLabels
    } = this.state;
    return (
      <Card>
        <div
          id="scene"
          className="position-relative"
          style={{ width: "100%", height: "400px" }}
          ref={(mount) => {
            this.mount = mount;
          }}
        >
          <div className="m-2" style={{ position: "absolute", textAlign: "left" }}>
            <SettingFilled
              className={`mb-2 ${optionsVisible ? "icon" : "icon-deactive"}`}
              style={{ fontSize: "16px" }}
              onClick={() => this.toggleOptions()}
            />

            <div className={optionsVisible ? "" : "d-none"}>
              <React.Fragment>
                <p className="m-0 mt-2 text-light">
                  <span>Perspective</span>
                </p>
                <div className="menu-switch" onClick={() => this.changeCameraPerspective()}>
                  <small className="mr-2 ml-2 mb-1">{currentPerspective}</small>
                </div>
              </React.Fragment>
              {this.renderToggleSwitch(
                "3D Frustum",
                () => this.setState({ frustum3d: !frustum3d }),
                frustum3d
              )}
              {this.renderToggleSwitch(
                "Object Labels",
                () => this.setState({ renderLabels: !renderLabels }),
                renderLabels
              )}
              {this.renderToggleSwitch("Axes", () => this.toggleHelperAxes(), helperAxesVisible)}
              {this.renderToggleSwitch("Grid", () => this.toggleGrid(), gridVisible)}
              <div className={gridVisible ? "" : "d-none"}>
                <p className="m-0 mt-2 text-light">
                  <span>
                    Grid Cellsize:
                    {` ${gridCellsize}m`}
                  </span>
                </p>
                <div className="rangeslider-horizontal-thin">
                  <Slider
                    tooltip={false}
                    value={gridCellsize}
                    onChange={this.handleGridCellsize}
                    min={0}
                    max={25}
                    step={0.5}
                  />
                </div>
              </div>
            </div>
          </div>
          <ReplayControlls
            replayState={replayState}
            streamingType={streamingType}
            isRecording={isRecording}
            toggleReplay={this.toggleReplay}
            fastForward={this.fastForward}
            rewind={this.rewind}
            recordCanvas={this.recordCanvas}
            isBuffering={isBuffering}
          ></ReplayControlls>
          <div
            className="m-0 mt-2 text-light"
            style={{
              position: "absolute",
              transform: "translate(25px, -0.5px)"
            }}
          >
            <UndoOutlined
              className="m-2 icon-white"
              style={{ color: "#2bbbad", fontSize: "16px" }}
              onClick={() => this.resetRenderOptions()}
            />
          </div>
        </div>
        {this.renderSensorSelect(this.sensorType.Object, selectObjSensors)}
        {this.renderSensorSelect(this.sensorType.Lane, selectLaneSensors)}
      </Card>
    );
  }

  renderToggleSwitch = (label, action, isChecked) => {
    return (
      <React.Fragment>
        <p className="m-0 mt-2 text-light" style={{ fontSize: "16px" }}>
          <span>{label}</span>
        </p>
        <ToggleSwitch onChange={action} checked={isChecked} height={10} />
      </React.Fragment>
    );
  };

  renderSensorSelect = (sensorType, selectSensors) => {
    let colors;
    let defaultColors;
    let defaultSensor;
    switch (sensorType) {
      case this.sensorType.Object:
        colors = this.objColors;
        defaultColors = this.defaultColors.Object;
        defaultSensor = this.defaultSensor.Object;
        break;
      case this.sensorType.Lane:
        colors = this.laneColors;
        defaultColors = this.defaultColors.Lane;
        defaultSensor = this.defaultSensor.Lane;
        break;
      default:
      // Invalid sensor type, but already checked elsewhere.
    }
    return (
      <div className="row">
        <Select
          mode="multiple"
          style={{
            width: "95.5%",
            marginTop: "6px",
            marginLeft: "16px"
          }}
          tagRender={(props) => {
            const { label, value, closable, onClose } = props;
            return (
              <Tag
                color={colors[value]}
                closable={closable}
                onClose={onClose}
                style={{
                  marginRight: 3,
                  color: "rgb(0,0,0)",
                  borderColor: "#999"
                }}
              >
                {label}
              </Tag>
            );
          }}
          placeholder={`Select ${sensorType} sensors`}
          defaultValue={[defaultSensor]}
          onChange={(value) => {
            for (const v of value) {
              if (!(v in colors)) {
                const k = Object.keys(colors).length;
                colors[v] =
                  k < defaultColors.length
                    ? defaultColors[k]
                    : randomColor({ luminosity: "light" });
              }
            }
            switch (sensorType) {
              case this.sensorType.Object:
                this.setState({ objSensorsToRender: value });
                break;
              case this.sensorType.Lane:
                this.setState({ laneSensorsToRender: value });
                break;
              default:
              // Invalid sensor type, but already checked elsewhere.
            }
          }}
        >
          {selectSensors.map((sensor) => (
            <Option key={sensor.id} value={sensor.value}>
              {sensor.value}
              <small className="ml-2 mr-2 text-dark mr-4">{sensor.name}</small>
            </Option>
          ))}
        </Select>
      </div>
    );
  };

  componentDidMount() {
    // Event listener is needed to ensure that the scene hast the proper size
    // if it's containing bootstrap column gets resized.
    window.addEventListener("resize", this.handleResize, false);

    // Get width and height of the containing element.
    const width = this.mount.clientWidth;
    const height = this.mount.clientHeight;

    // Create needed THREE objects.
    const renderer = new THREE.WebGLRenderer({ antialias: true });
    const camera = new THREE.PerspectiveCamera(45, width / height, 0.1, 10000);
    camera.up.set(0, 0, 1);
    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableKeys = false;

    // Move and rotate camera to viewing position.
    this.cameraSetPosition(camera, this.defaultCameraPerspective);
    controls.update();

    // Create and configure scene.
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x1c2331);
    scene.add(camera);
    renderer.setSize(width, height);
    document.querySelector("#scene").appendChild(renderer.domElement);

    // Create ground.
    const geometry = this.createGridGeometry(1);
    const material = new THREE.LineBasicMaterial({
      color: 0x3b3b3b
    });
    const grid = new THREE.Line(geometry, material, THREE.LineSegments);
    grid.position.set(0, 0, -0.005);
    scene.add(grid);

    // Create lighting for the scene.
    const ambientLight = new THREE.AmbientLight(0xffffff, 1);
    scene.add(ambientLight);

    // Create axes.
    const axes = new THREE.AxesHelper(200);
    this.axes = axes;

    // Render the scene.
    renderer.render(scene, camera);

    // Assign scene objects to class.
    this.renderer = renderer;
    this.scene = scene;
    this.camera = camera;
    this.grid = grid;
    this.controls = controls;
    this.start();
    this._isMounted = true;
  }

  getAvailSensors = (sensors, sensorType) => {
    const selectSensors = [];
    switch (sensorType) {
      case this.sensorType.Object:
        for (const sensor in sensors) {
          if (sensors[sensor].sensed_objects) {
            selectSensors.push(this.addSensorToList(sensor, sensors[sensor]));
          }
        }
        break;
      case this.sensorType.Lane:
        for (const sensor in sensors) {
          if (sensors[sensor].sensed_lane_boundaries) {
            selectSensors.push(this.addSensorToList(sensor, sensors[sensor]));
          }
        }
        break;
      default:
        console.error("Unexpected sensor type!", sensorType);
    }
    return selectSensors;
  };

  addSensorToList = (item, sensor) => {
    return { id: sensor.id, value: item, name: sensor.name };
  };

  updateSensedEnvironment = () => {
    const { sensors } = this.props;

    // Remove the temporary objects from last rendering.
    this.removeTemporaryObjectsFromScene(this.scene);

    // Ego vehicle data.
    this.updateEgoVehicle(sensors);

    // Static and dynamic objects.
    this.updateWorldObjects(sensors);

    // Lane boundaries.
    this.updateLaneBoundaries(sensors);
  };

  removeTemporaryObjectsFromScene = (scene) => {
    const temporaryObjects = scene.children.filter((child) => child.temporary === true);
    for (const lineIndex in temporaryObjects) {
      scene.remove(temporaryObjects[lineIndex]);
    }
  };

  updateEgoVehicle = (sensors) => {
    const egoSensor = (sensors || {})[this.defaultSensor.Ego];
    if (
      this.scene.children.find((x) => x.vehicleKey === "ego") === undefined &&
      egoSensor !== undefined &&
      egoSensor.sensed_state !== null
    ) {
      // Create ego vehicle and place it in scene if not done yet.
      this.createEgoVehicle(egoSensor.sensed_state);
    } else if (egoSensor) {
      this.deleteEgoVehicle();
      this.createEgoVehicle(egoSensor.sensed_state);
    }
  };

  updateWorldObjects = (sensors) => {
    const { objSensorsToRender } = this.state;
    const worldSensor = (sensors || {})[this.defaultSensor.Object];
    // Create the world sensors's frustum.
    if (worldSensor) {
      if (worldSensor.frustum) {
        const frustum = this.createFrustum(worldSensor.frustum, worldSensor.mount_pose);
        this.scene.add(frustum);
      }
    }
    // Only consider sensors that were requested by the user.
    this.addWorldObjects(sensors, objSensorsToRender);

    // Remove deprecated detected objects.
    this.removeDeprecatedObjects(sensors, objSensorsToRender);
  };

  addWorldObjects = (allSensors, renderSensors) => {
    for (const name of renderSensors) {
      const sensor = (allSensors || {})[name];
      if (!sensor) {
        continue;
      }
      // Add objects.
      for (const object of sensor.sensed_objects) {
        // Check if object is already existing.
        if (!this.scene.children.find((x) => x.detectedBy === name && x.vehicleID === object.id)) {
          // Create new vehicle if no vehicle with the current object id is found.
          this.placeWorldObject(object, name, sensor);
        } else {
          // If vehicle is already existent, update it's position.
          this.updateWorldObject(object, name, sensor);
        }
      }
    }
  };

  removeDeprecatedObjects = (allSensors, renderSensors) => {
    for (const object of this.scene.children) {
      if (!object.detectedBy) {
        continue;
      }
      const sensor = (allSensors || {})[object.detectedBy];
      // Check if sensor is available and if its data is supposed to be rendered.
      let deleteObj = !sensor || !renderSensors.find((x) => x === object.detectedBy);
      if (!deleteObj && sensor) {
        // Check if object is seen by the sensor.
        deleteObj = !sensor.sensed_objects.find((x) => x.id === object.vehicleID);
      }
      if (deleteObj) {
        this.scene.remove(object.wireFrame);
        this.scene.remove(object);
      }
    }
  };

  updateLaneBoundaries = (sensors) => {
    const { laneSensorsToRender } = this.state;
    for (const name of laneSensorsToRender) {
      const sensor = (sensors || {})[name];
      if (!sensor) {
        continue;
      }
      this.updateSensedLanes(sensor, name);
    }
  };

  createEgoVehicle = (ego) => {
    const vehicleGeometry = this.createBoxGeometry(ego);
    vehicleGeometry.applyMatrix4(
      new THREE.Matrix4().makeTranslation(ego.cog_offset.x, ego.cog_offset.y, ego.dimensions.z / 2)
    );

    const egoBox = new THREE.Mesh(vehicleGeometry, this.egoVehicleMaterial);
    egoBox.up.set(0, 0, 1);
    egoBox.vehicleKey = "ego";

    const egoWireFrame = this.createLineSegments(vehicleGeometry);
    egoBox.add(egoWireFrame);
    this.scene.add(egoBox);
  };

  deleteEgoVehicle = () => {
    const ego = this.scene.children.find((x) => x.vehicleKey === "ego");
    const cone = this.scene.children.find((x) => x.vehicleKey === "sensor");
    this.scene.remove(ego);
    this.scene.remove(cone);
  };

  placeWorldObject = (object, detectedBy, sensor) => {
    const material = new THREE.MeshLambertMaterial({
      color: this.objColors[detectedBy],
      transparent: true,
      opacity: 0.5
    });
    const geometry = this.createBoxGeometry(object);

    const vehicle = new THREE.Mesh(geometry, material);
    vehicle.up.set(0, 0, 1);
    vehicle.detectedBy = detectedBy;
    vehicle.vehicleID = object.id;

    this.updateWorldObjectPosition(sensor, object, vehicle);
    this.updateWorldObjectRotation(sensor, object, vehicle);

    const vehicleWireFrame = this.createLineSegments(geometry);
    vehicle.wireFrame = vehicleWireFrame;
    vehicle.add(vehicleWireFrame);
    this.scene.add(vehicle);
  };

  updateWorldObject = (object, detectedBy, sensor) => {
    const vehicle = this.scene.children.find(
      (x) => x.detectedBy === detectedBy && x.vehicleID === object.id
    );
    this.updateWorldObjectDimensions(object, vehicle);
    this.updateWorldObjectPosition(sensor, object, vehicle);
    this.updateWorldObjectRotation(sensor, object, vehicle);
    if (this.state.renderLabels) {
      const label = this.getObjectLabel(object, vehicle, this.objColors[detectedBy]);
      this.scene.add(label);
    }
  };

  updateWorldObjectDimensions = (object, vehicle) => {
    const geometry = this.createBoxGeometry(object);
    vehicle.geometry.dispose();
    vehicle.geometry = geometry;
    const vehicleWireFrame = this.createLineSegments(geometry);
    vehicle.wireFrame.geometry.dispose();
    vehicle.remove(vehicle.wireFrame);
    vehicle.wireFrame = vehicleWireFrame;
    vehicle.add(vehicleWireFrame);
  };

  updateWorldObjectPosition = (sensor, object, vehicle) => {
    const location = new THREE.Vector3(
      object.cog_offset.x,
      object.cog_offset.y,
      object.dimensions.z / 2
    );
    // Correct the reference point of the detected object as follows:
    // objPos(sensorFrame) = dObj(sensorFrame) +
    //                       RotMat(obj, sensorFrame) * dCog(objectFrame)
    location.applyMatrix4(this.getTransfMatrixFromFramePose(object.pose));
    if (sensor.mount_pose) {
      // Transform the detected object pose as follows:
      // objPos(egoFrame) = dSensor(egoFrame) +
      //                    RotMat(sensor, egoFrame) * objPos(sensorFrame)
      location.applyMatrix4(this.getTransfMatrixFromFramePose(sensor.mount_pose));
    }
    vehicle.position.set(location.x, location.y, location.z);
  };

  updateWorldObjectRotation = (sensor, object, vehicle) => {
    const quaternion = this.getQuaternionFromPose(object.pose);
    if (sensor.mount_pose) {
      // Convert object rotation from sensor frame into ego frame.
      const sensorQuat = this.getQuaternionFromPose(sensor.mount_pose);
      quaternion.multiply(sensorQuat);
    }
    vehicle.setRotationFromQuaternion(quaternion);
  };

  updateSensedLanes = (sensor, detectedBy) => {
    const lanes = sensor.sensed_lane_boundaries;
    for (const laneIndex in lanes) {
      const lane = lanes[laneIndex];
      const pointsEgoFrame = [];
      const line = this.getLaneGeomFromGtPoints(lane, sensor, pointsEgoFrame);
      const lineC = this.getLaneGeomFromClothoid(lane, detectedBy, pointsEgoFrame);
      this.addLineToScene(line);
      this.addLineToScene(lineC);
      if (this.state.renderLabels) {
        const label = this.getLaneBoundaryLabel(lane, pointsEgoFrame, this.laneColors[detectedBy]);
        this.scene.add(label);
      }
    }
  };

  getLaneGeomFromGtPoints = (lane, sensor, pointsEgoFrame) => {
    const geometry = new THREE.BufferGeometry();
    const material =
      lane.type === "dashed"
        ? new THREE.LineDashedMaterial({
            linewidth: 1,
            color: 0xffffff,
            dashSize: 3,
            gapSize: 3
          })
        : new THREE.LineBasicMaterial({ color: 0xffffff });
    for (const point of lane.points) {
      const pointEgoFrame = new THREE.Vector3(point.x, point.y, point.z);
      if (sensor.mount_pose) {
        // Transform the lane coordinates as follows:
        // point(egoFrame) = dSensor(egoFrame) +
        //                   RotMat(sensor, egoFrame) * point(sensorFrame)
        pointEgoFrame.applyMatrix4(this.getTransfMatrixFromFramePose(sensor.mount_pose));
      }
      pointsEgoFrame.push(pointEgoFrame);
    }
    geometry.setFromPoints(pointsEgoFrame);
    return new THREE.Line(geometry, material);
  };

  getLaneGeomFromClothoid = (lane, detectedBy, pointsEgoFrame) => {
    const geometry = new THREE.BufferGeometry();
    const material = new THREE.LineBasicMaterial({
      color: this.laneColors[detectedBy],
      linewidth: 2
    });
    const n = pointsEgoFrame.length;
    const lineLengthGT = pointsEgoFrame[n - 1].x - pointsEgoFrame[0].x;
    if (lineLengthGT < 0.01) {
      // If the points fall within a distance of 1cm, skip the clothoid rendering.
      geometry.setFromPoints([new THREE.Vector3(0.0, 0.0, 0.0)]);
      return new THREE.Line(geometry, material);
    }
    const clothoid = {
      dxStart: lane.dx_start,
      dyStart: lane.dy_start,
      dxEnd: lane.dx_end,
      curvStart: lane.curv_hor_start,
      curvChange: lane.curv_hor_change,
      headingStart: lane.heading_start,
      oneOvSix: 1.0 / 6.0
    };
    // Estimate the point sampling distance.
    const ds = lineLengthGT / (2 * (n + 1));
    const vertices = [];
    // Compute clothoid points.
    for (let s = clothoid.dxStart; s < clothoid.dxEnd; s += ds) {
      const point = new THREE.Vector3(s, 0.0, this.interpPointZ(pointsEgoFrame, s));
      vertices.push(this.calcClothoidPoint(clothoid, point));
    }
    const point = new THREE.Vector3(
      clothoid.dxEnd,
      0.0,
      this.interpPointZ(pointsEgoFrame, clothoid.dxEnd)
    );
    vertices.push(this.calcClothoidPoint(clothoid, point));
    geometry.setFromPoints(vertices);
    return new THREE.Line(geometry, material);
  };

  addLineToScene = (line) => {
    line.computeLineDistances();
    line.temporary = true;
    this.scene.add(line);
  };

  interpPointZ = (points, pointX) => {
    let ind = points.findIndex((pt) => pt.x > pointX);
    ind = ind >= 0 ? ind : points.length - 1;
    let ind1 = -1;
    if (ind === 0) {
      ind1 = ind + 1;
    } else {
      ind1 = ind - 1;
    }
    const m = (points[ind1].z - points[ind].z) / (points[ind1].x - points[ind].x);
    return points[ind].z + m * (pointX - points[ind].x);
  };

  calcClothoidPoint = (c, point) => {
    // cf. Dickmanns and Zapp, SPIE Vol. 727 Mobile Robots (1986), 161-168.
    const l = point.x; // Small angle approximation: x ~= arc length
    const yC =
      c.dyStart +
      l * (Math.tan(c.headingStart) + l * (0.5 * c.curvStart + c.oneOvSix * c.curvChange * l));
    return new THREE.Vector3(point.x, yC, point.z);
  };

  getObjectLabel = (object, vehicle, color) => {
    const text = this.createDefaultLabelText(object);
    const label = this.createDefaultLabel(text, color);
    // Set the label position.
    const pos = vehicle.position;
    label.position.set(pos.x, pos.y, pos.z + object.dimensions.z);
    return label;
  };

  getLaneBoundaryLabel = (lane, points, color) => {
    const text = this.createDefaultLabelText(lane);
    const label = this.createDefaultLabel(text, color);
    // Set the label position.
    const ind = points.findIndex((pt) => pt.x > 10);
    label.position.set(points[ind].x, points[ind].y, points[ind].z + 1.0);
    return label;
  };

  createDefaultLabelText = (object) => {
    let existProb = 1.0;
    if (object.exist_prob !== undefined) {
      existProb = object.exist_prob;
    }
    return `ID: ${object.id} \n Prob.: ${existProb.toFixed(2)}`;
  };

  createDefaultLabel = (text, color) => {
    const label = new SpriteText(text);
    label.fontSize = 90;
    label.textHeight = 0.12;
    label.borderWidth = 0.5 * label.textHeight;
    label.padding = [4.0 * label.textHeight, 4.0 * label.textHeight];
    label.color = color;
    label.backgroundColor = "#1C2833";
    label.temporary = true;
    return label;
  };

  createFrustum = (frustum, pose) => {
    // Sensor information:
    const sensorPosition = this.createVectorFromPose(pose);
    const angleVertical = frustum.fov_v;
    const angleHorizontal = frustum.fov_h;
    const clipNearDis = frustum.clip_near;
    const clipFarDis = frustum.clip_far;
    // Offset of frustum is not considered yet.
    if (frustum.offset_h + frustum.offset_v !== 0) {
      console.error("Frustum offset is not 0!", frustum);
    }
    const radNearVertical = this.tangensFromRadiants(angleVertical / 2) * clipNearDis;
    const radFarVertical = this.tangensFromRadiants(angleVertical / 2) * clipFarDis;
    const radNearHorizontal = this.tangensFromRadiants(angleHorizontal / 2) * clipNearDis;
    const radFarHorizontal = this.tangensFromRadiants(angleHorizontal / 2) * clipFarDis;

    let clipNear = [];
    let clipFar = [];
    const groundLevel = 0.1;

    if (this.state.frustum3d) {
      clipNear = [
        new THREE.Vector3(clipNearDis, radNearHorizontal, -radNearVertical).add(sensorPosition),

        new THREE.Vector3(clipNearDis, radNearHorizontal, radNearVertical).add(sensorPosition),

        new THREE.Vector3(clipNearDis, -radNearHorizontal, radNearVertical).add(sensorPosition),

        new THREE.Vector3(
          clipNearDis,
          -radNearHorizontal,
          Math.max(-groundLevel, -radNearVertical)
        ).add(sensorPosition)
      ];

      clipFar = [
        new THREE.Vector3(
          clipFarDis,
          radFarHorizontal,
          Math.max(-groundLevel, -radFarVertical)
        ).add(sensorPosition),

        new THREE.Vector3(clipFarDis, radFarHorizontal, radFarVertical).add(sensorPosition),

        new THREE.Vector3(clipFarDis, -radFarHorizontal, radFarVertical).add(sensorPosition),

        new THREE.Vector3(
          clipFarDis,
          -radFarHorizontal,
          Math.max(-groundLevel, -radFarVertical)
        ).add(sensorPosition)
      ];
    } else {
      clipNear = [
        new THREE.Vector3(clipNearDis, radNearHorizontal, groundLevel).add(sensorPosition),
        new THREE.Vector3(clipNearDis, -radNearHorizontal, groundLevel).add(sensorPosition)
      ];
      clipFar = [
        new THREE.Vector3(clipFarDis, radFarHorizontal, groundLevel).add(sensorPosition),
        new THREE.Vector3(clipFarDis, -radFarHorizontal, groundLevel).add(sensorPosition)
      ];
    }

    const frustumPoints = clipNear.concat(clipFar);
    var frustumBufferGeometry = new THREE.BufferGeometry();
    frustumBufferGeometry.setFromPoints(frustumPoints);
    const vertices = [];
    // Define vertex positions.
    if (this.state.frustum3d) {
      // prettier-ignore
      vertices.push(
        frustumPoints[0].x, frustumPoints[0].y, frustumPoints[0].z,
        frustumPoints[1].x, frustumPoints[1].y, frustumPoints[1].z,
        frustumPoints[2].x, frustumPoints[2].y, frustumPoints[2].z,
        frustumPoints[3].x, frustumPoints[3].y, frustumPoints[3].z,
        frustumPoints[4].x, frustumPoints[4].y, frustumPoints[4].z,
        frustumPoints[5].x, frustumPoints[5].y, frustumPoints[5].z,
        frustumPoints[6].x, frustumPoints[6].y, frustumPoints[6].z,
        frustumPoints[7].x, frustumPoints[7].y, frustumPoints[7].z
      );
    } else {
      // prettier-ignore
      vertices.push(
        frustumPoints[0].x, frustumPoints[0].y, frustumPoints[0].z,
        frustumPoints[1].x, frustumPoints[1].y, frustumPoints[1].z,
        frustumPoints[2].x, frustumPoints[2].y, frustumPoints[2].z,
        frustumPoints[3].x, frustumPoints[3].y, frustumPoints[3].z
      );
    }

    // Number of attribute components per vertex (position or color).
    const itemSize = 3;
    // Set position of frustum geometry.
    frustumBufferGeometry.setAttribute(
      "position",
      new THREE.BufferAttribute(new Float32Array(vertices), itemSize)
    );
    // Define faces via index.
    if (this.state.frustum3d) {
      // prettier-ignore
      frustumBufferGeometry.setIndex([
        2, 1, 0, 0, 3, 2, 1, 5, 4,
        1, 4, 0, 1, 6, 5, 2, 6, 1,
        3, 6, 2, 3, 7, 6, 0, 7, 3,
        4, 7, 0, 4, 5, 6, 4, 6, 7,
      ]);
    } else {
      // prettier-ignore
      frustumBufferGeometry.setIndex([
        0, 1, 2,
        0, 3, 2,
      ]);
    }
    // Set uniform color of frustum geometry.
    const colors = [];
    Array.from({ length: vertices.length / itemSize }, () => colors.push(1, 0.98, 0.56));
    // Map RGB color to vertices.
    frustumBufferGeometry.setAttribute(
      "color",
      new THREE.BufferAttribute(new Float32Array(colors), itemSize)
    );

    // Compute geometric properties.
    frustumBufferGeometry.computeVertexNormals();
    frustumBufferGeometry.computeBoundingSphere();

    const frustumToRender = new THREE.Mesh(frustumBufferGeometry, this.sensorMaterial);
    frustumToRender.up.set(0, 0, 1);

    const quaternion = this.getQuaternionFromPose(pose);
    frustumToRender.setRotationFromQuaternion(quaternion);

    frustumToRender.temporary = true;

    return frustumToRender;
  };

  createBoxGeometry = (object) => {
    return new THREE.BoxGeometry(object.dimensions.x, object.dimensions.y, object.dimensions.z);
  };

  createLineSegments = (boxGeom) => {
    const wfGeometry = new THREE.EdgesGeometry(boxGeom);
    return new THREE.LineSegments(wfGeometry, this.wfMaterial);
  };

  createVectorFromPose = (pose) => {
    return new THREE.Vector3(pose.translation.x, pose.translation.y, pose.translation.z);
  };

  getQuaternionFromPose = (pose) => {
    return new THREE.Quaternion(pose.rotation.x, pose.rotation.y, pose.rotation.z, pose.rotation.w);
  };

  getTransfMatrixFromFramePose = (pose) => {
    const quaternion = this.getQuaternionFromPose(pose);
    const translation = new THREE.Matrix4().makeTranslation(
      pose.translation.x,
      pose.translation.y,
      pose.translation.z
    );
    const rotMat = new THREE.Matrix4().makeRotationFromQuaternion(quaternion);
    return translation.multiply(rotMat);
  };

  createGridGeometry = (step) => {
    const size = 1000;
    const geometry = new THREE.BufferGeometry();
    const vertices = [];
    for (let i = -size; i <= size; i += step) {
      vertices.push(new THREE.Vector3(-size, i, 0));
      vertices.push(new THREE.Vector3(size, i, 0));
      vertices.push(new THREE.Vector3(i, -size, 0));
      vertices.push(new THREE.Vector3(i, size, 0));
    }
    geometry.setFromPoints(vertices);
    return geometry;
  };

  componentWillUnmount() {
    this.stop();
    this.mount.removeChild(this.renderer.domElement);
    window.removeEventListener("resize", this.handleResize, false);
  }

  handleResize = () => {
    this.camera.aspect = this.mount.clientWidth / this.mount.clientHeight;
    this.camera.updateProjectionMatrix();
    this.renderer.setSize(this.mount.clientWidth, this.mount.clientHeight);
  };

  start() {
    if (!this.frameId) {
      this.frameId = requestAnimationFrame(this.animate);
    }
  }

  stop() {
    cancelAnimationFrame(this.frameId);
  }

  animate() {
    this.controls.update();
    this.renderScene();
    this.frameId = window.requestAnimationFrame(this.animate);
  }

  renderScene() {
    this.renderer.render(this.scene, this.camera);
  }

  // Make rendering options un-/visible.
  toggleOptions = () => {
    this.setState({ optionsVisible: !this.state.optionsVisible });
  };

  toggleReplay = (isRunning) => {
    this.props.toggleReplay(isRunning);
  };

  fastForward = () => {
    this.props.fastForward();
  };

  rewind = () => {
    this.props.rewind();
  };

  recordCanvas = () => {
    this.props.recordCanvas();
  };

  resetRenderOptions = () => {
    if (this.state.helperAxesVisible) {
      this.scene.remove(this.axes);
    }
    if (!this.state.gridVisible) {
      this.scene.add(this.grid);
    }
    this.setState({
      helperAxesVisible: false,
      gridVisible: true,
      gridCellsize: 1,
      currentPerspective: "Default",
      frustum3d: false
    });
    this.cameraSetPosition(this.controls.object, this.defaultCameraPerspective);
    this.cameraSetTarget(this.controls, this.defaultCameraPerspective);
    this.handleGridCellsize(1);
  };

  // Make helper axes un-/visible.
  toggleHelperAxes = () => {
    if (this.state.helperAxesVisible) {
      this.scene.remove(this.axes);
    } else {
      this.scene.add(this.axes);
    }
    this.setState({ helperAxesVisible: !this.state.helperAxesVisible });
  };

  // Make grid un-/visible.
  toggleGrid = () => {
    if (this.state.gridVisible) {
      this.scene.remove(this.grid);
    } else {
      this.scene.add(this.grid);
    }
    this.setState({ gridVisible: !this.state.gridVisible });
  };

  // Change the grid's cellsize in state to the new value,
  // trigger the update for the grid's geometry.
  handleGridCellsize = (value) => {
    this.setState({ gridCellsize: value }, this.changeGridCellsize(value));
  };

  // Change geometry of the grid to a given stepsize.
  changeGridCellsize = (size) => {
    const cellSize = size !== 0 ? size : 2000;
    this.grid.geometry = this.createGridGeometry(cellSize);
  };

  changeCameraPerspective = () => {
    const index =
      this.currentCameraPerspective < this.cameraPerspecives.length - 1
        ? this.currentCameraPerspective + 1
        : 0;
    this.setState({ currentPerspective: this.cameraPerspecives[index].name });
    this.currentCameraPerspective = index;
    this.cameraSetPosition(this.controls.object, index);
    this.cameraSetTarget(this.controls, index);
  };

  cameraSetPosition = (camera, ind) => {
    camera.position.set(
      this.cameraPerspecives[ind].camera.x,
      this.cameraPerspecives[ind].camera.y,
      this.cameraPerspecives[ind].camera.z
    );
  };

  cameraSetTarget = (controls, ind) => {
    controls.target.set(
      this.cameraPerspecives[ind].target.x,
      this.cameraPerspecives[ind].target.y,
      this.cameraPerspecives[ind].target.z
    );
  };

  tangensFromRadiants = (rad) => {
    return Math.tan(rad);
  };
}

export default Rendering;
