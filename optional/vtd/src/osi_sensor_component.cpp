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
 * \file osi_sensor_component.cpp
 * \see  osi_sensor_component.hpp
 */

#include "osi_sensor_component.hpp"

#include <sstream>  // for istringstream
#include <string>   // for string

#include <boost/math/constants/constants.hpp>  // for pi
#include <boost/property_tree/ptree.hpp>       // for ptree
#include <boost/property_tree/xml_parser.hpp>  // for read_xml

#include <cloe/simulator.hpp>  // for ModelError

#include <osi/utility/osi_omni_sensor.hpp>  // for SensorMockTarget
#include "vtd_conf.hpp"                     // for VtdSensorConfig

namespace vtd {

void VtdOsiSensor::configure(const VtdSensorConfig& cfg) {
  boost::property_tree::ptree pt;
  std::istringstream is(cfg.xml);
  boost::property_tree::read_xml(is, pt);
  boost::property_tree::ptree child;

  // In VTD v2.2, the sensor mounting position is not provided as OSI message.
  // Hence, the coordinate reference frame transformations rely on the
  // configuration input stored in vtd_mnt_pos_dxyz_ and vtd_mnt_ori_drpy_.
  child = pt.get_child("Sensor.Position");
  vtd_mnt_pos_dxyz_(0) = child.get<double>("<xmlattr>.dx");
  vtd_mnt_pos_dxyz_(1) = child.get<double>("<xmlattr>.dy");
  vtd_mnt_pos_dxyz_(2) = child.get<double>("<xmlattr>.dz");

  auto deg_to_rad = [](const double deg) {
    return deg / 180 * boost::math::constants::pi<double>();
  };
  vtd_mnt_ori_drpy_(0) = deg_to_rad(child.get<double>("<xmlattr>.drDeg"));
  vtd_mnt_ori_drpy_(1) = deg_to_rad(child.get<double>("<xmlattr>.dpDeg"));
  vtd_mnt_ori_drpy_(2) = deg_to_rad(child.get<double>("<xmlattr>.dhDeg"));

  child = pt.get_child("Sensor.Frustum");
  frustum_.clip_near = child.get<double>("<xmlattr>.near");
  frustum_.clip_far = child.get<double>("<xmlattr>.far");
  double ang_deg = child.get<double>("<xmlattr>.left");
  double fov_deg = ang_deg + child.get<double>("<xmlattr>.right");
  frustum_.fov_h = deg_to_rad(fov_deg);
  frustum_.offset_h = deg_to_rad(ang_deg) - 0.5 * frustum_.fov_h;
  ang_deg = child.get<double>("<xmlattr>.bottom");
  fov_deg = ang_deg + child.get<double>("<xmlattr>.top");
  frustum_.fov_v = deg_to_rad(fov_deg);
  frustum_.offset_v = deg_to_rad(ang_deg) - 0.5 * frustum_.fov_v;

  child = pt.get_child("Sensor.Origin");
  if (child.get<std::string>("<xmlattr>.type") != "sensor") {
    throw cloe::ModelError(
        "OSI interface only supports object detection in sensor coordinate reference frame "
        "(Sensor.Origin type=sensor)");
  }

  set_mock_conf(cfg.sensor_mock_conf);
}

}  // namespace vtd
