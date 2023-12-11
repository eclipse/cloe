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
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * \file scp_messages.cpp
 * \see  scp_messages.hpp
 */

#include <string>

#include "scp_messages.hpp"  // NOLINT

namespace vtd::scp {
// clang-format off

const char* Start = "<SimCtrl><Start/></SimCtrl>";
const char* Stop = "<SimCtrl><Stop/></SimCtrl>";
const char* Pause = "<SimCtrl><Pause/></SimCtrl>";
const char* Restart = "<SimCtrl><Restart/></SimCtrl>";
const char* Apply = "<SimCtrl><Apply/></SimCtrl>";
const char* Config = "<SimCtrl><Config/></SimCtrl>";
const char* QueryInit = "<Query entity=\"taskControl\"><Init source=\"cloe\" /></Query>";
const char* AckInit = "<SimCtrl><InitDone source=\"cloe\" /></SimCtrl>";
const char* InitOperation = "<SimCtrl><Init mode=\"operation\" /></SimCtrl>";

std::string ParamServerConfig::to_scp() const {
  std::string tc_config = fmt::format(R"SCP(
    <Sync>
      <source value="{}"/>
      <realTime value="false"/>
      <waitAfterFrame value="true"/>
    </Sync>
  )SCP", sync_source);

  if(no_image_generator) {
    tc_config += fmt::format(R"SCP(
    <ImageGenerator>
      <ignore value="true"/>
      <imgPortConnect value="false"/>
      <ctrlPortConnect value="false"/>
    </ImageGenerator>
    )SCP", sync_source);
  }
  return fmt::format(R"SCP(
    <ParamServer>
      <Submit component="Cloe" target="TaskControl" action="modify">
        <TaskControl xmlns="http://www.vires.com/2015/VtdParamSchema/TaskControl">
          {}
        </TaskControl>
      </Submit>
    </ParamServer>
    )SCP", tc_config);
}

std::string ScenarioConfig::to_scp() const {
  return fmt::format(R"SCP(
    <SimCtrl>
      <UnloadSensors />
      <LoadScenario filename="{}" />
    </SimCtrl>
    )SCP", filename);
}

std::string CameraPosition::to_scp() const {
  assert(!tethered_to_player.empty());
  assert(!look_to_player.empty());

  return fmt::format(R"SCP(
    <Camera name='tethered_CAMERA' showOwner='true'>
      <Frustum
        far='1500.000000'
        fovHor='40.000000'
        fovVert='30.000000'
        near='1.000000'
        offsetHor='0.000000'
        offsetVert='0.000000'
        />
      <PosTether
        azimuth='0.174533'
        distance='19.000000'
        elevation='0.226893'
        slew ='100'
        player='{}'
        />
      <ViewPlayer player='{}' />
      <Set/>
    </Camera>
  )SCP", tethered_to_player, look_to_player);
}

std::string SensorConfiguration::to_scp() const {
  assert(player_id >= 0);
  assert(port > 0 && port < 70000);

  return fmt::format(R"SCP(
    <Sensor name='PerfectSensor_{0}' type='video'>
      <Load
        lib='libModulePerfectSensor.so'
        path=''
        persistent='true'
        />
      <Frustum
        near='0.0'
        far='180.0'
        left='180.0'
        right='180.0'
        bottom='180.0'
        top='180.0'
        />
      <Origin type='sensor' />
      <Cull maxObjects='50' enable='true' />
      <Port name='RDBout' number='{1}' type='TCP' sendEgo='true' />
      <Player id="{2}" />
      <Position dx='0.0' dy='0.0' dz='0.0' dhDeg='0.0' dpDeg='0.0' drDeg='0.0' />
      <Database resolveRepeatedObjects='true' continuousObjectTesselation='2.0' />
      <Filter objectType='pedestrian'/>
      <Filter objectType='vehicle'/>
      <Filter objectType='trafficSign'/>
      <Filter objectType='obstacle'/>
      <Filter
        objectType="roadMarks"
        roadmarkPreviewDistance="100.0"
        tesselate="true"
        tesselateNoPoints="10"
        tesselateFixedStep="true"
        />
      <Debug
        enable='false'
        detection='false'
        road='false'
        position='true'
        dimensions='false'
        camera='false'
        CSV='false'
        packages='false'
        culling='true'
        contactPoints='false'
        trafficSigns='false'
        />
    </Sensor>)SCP", sensor_id, port, player_id);
}

std::string DynamicsPluginConfig::to_scp() const {
  return fmt::format(R"SCP(
    <DynamicsPlugin name="viTrafficDyn_{0}" enable="true">
      <Load     lib="libModuleTrafficDyn.so" path=""/>
      <Player   name="{0}" />
      <Debug    enable="false" />
    </DynamicsPlugin>)SCP", name);
}

std::string LabelVehicle::to_scp() const {
  return fmt::format(R"SCP(
  <Symbol name="{0}">
    <Text data="{1}" colorRGB="{2}"/>
    <PosPlayer player="{0}" dz="{3}"/>
  </Symbol>)SCP", tethered_to_player, text, color, dz_m);
}

std::string RecordDat::to_scp() const {
  return fmt::format(R"SCP(
  <Record>
    <File path="{0}" name="{1}" overwrite="true"/>
    <Start/>
  </Record>)SCP", datfile_path.parent_path().string(), datfile_path.filename().string());
}

std::string QueryScenario::to_scp() const {
  return fmt::format(R"SCP(
    <Query entity="traffic">
      <GetScenario filename="{0}"/>
    </Query>)SCP", scenario);
}

// clang-format on
}  // namespace vtd::scp
