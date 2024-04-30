/*
 * Copyright 2024 Robert Bosch GmbH
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
 * \file cloe/utility/frustum_culling.hpp
 */

#pragma once

#include <cmath>  // for cos, sin

#include <fmt/format.h>                // for format
#include <Eigen/Geometry>              // for Vector3d
#include <cloe/component/frustum.hpp>  // for Frustum

namespace cloe::utility {

struct Point {
  double x{0.0};
  double y{0.0};
};

// Rotates a given point from one coordinate system to another and returns rotated point
Point rotate_point(const Point& point, double angle) {
  return Point{std::cos(angle) * point.x - std::sin(angle) * point.y,
               std::sin(angle) * point.x + std::cos(angle) * point.y};
}

// Calculates the corner points of a field of view.
// p0 to p2 are the points counter clock-wise with the idstance clip_far to the root
// An additional offset of the field of view to the original coordinate coordinate system is considered.
//
//   clip_far  p2               p1
//                  \         /
//            x      \       /
//            ^       \     /
//            |        \   /
//      y <---|         \ /
//                      p0
// \param fov       Angle [radians] between p0-p1 and p0-p2
// \param offset    Angle [radians] to shift the points p1 and p2 by
// \param clip_far  Distance [meters] from p0 to p1 and from p0 to p2
//
// \return          Vector of points p0, p1, p2.
std::vector<Point> calc_corner_points(double fov, double offset, double clip_far) {
  std::vector<Point> ret_points;

  // initialize points
  Point p0{0.0, 0.0};
  Point p1{p0};
  Point p2{p0};

  ret_points.push_back(p0);

  p1.x = clip_far * std::cos(-fov / 2.0);
  p2.x = clip_far * std::cos(fov / 2.0);

  p1.y = clip_far * std::sin(-fov / 2.0);
  p2.y = clip_far * std::sin(fov / 2.0);

  p1 = rotate_point(p1, offset);
  p2 = rotate_point(p2, offset);

  ret_points.push_back(p1);
  ret_points.push_back(p2);

  return ret_points;
}

// Returns true if a given Point c is "on the left" of the line of two points a and b.
// "On the left" means the angle from the line a-b to the line a-c is in the range (0, M_PI)
bool is_left(Point a, Point b, Point c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0;
}

bool is_inside_fov(double fov, bool is_left_0_p1, bool is_left_0_p2, std::string error_message) {
  bool is_inside_fov = false;
  if (fov >= M_PI && fov <= 2 * M_PI) {
    // if the opening angle is between PI and 2 PI (180 and 360 degree)
    // then everything is inside the fov that is _not_ right of p1 AND is _not_ left of p2
    is_inside_fov = !(!is_left_0_p1 && is_left_0_p2);
  } else if (fov > 0.0 && fov < M_PI) {
    // if the opening angle is greater 0 and smaller than M_PI
    is_inside_fov = is_left_0_p1 && !is_left_0_p2;
  } else {
    throw std::runtime_error(error_message);
  }
  return is_inside_fov;
}

bool is_point_inside_frustum(const cloe::Frustum& frustum, const Eigen::Vector3d& point) {
  bool ret_val{false};

  // calculate points in "frustum" sensor coordinate system, which starts at the frustum root
  // and has x in viewing direction, and y to the left and z in the up direction.
  auto corner_points_x_y_plane =
      calc_corner_points(frustum.fov_h, frustum.offset_h, frustum.clip_far);
  auto corner_points_x_z_plane =
      calc_corner_points(frustum.fov_v, frustum.offset_v, frustum.clip_far);

  // calculate for xy plane
  bool is_left_0_p1 =
      is_left(corner_points_x_y_plane[0], corner_points_x_y_plane[1], Point{point.x(), point.y()});
  bool is_left_0_p2 =
      is_left(corner_points_x_y_plane[0], corner_points_x_y_plane[2], Point{point.x(), point.y()});

  bool is_inside_fov_xy =
      is_inside_fov(frustum.fov_h, is_left_0_p1, is_left_0_p2,
                    fmt::format("The field of view in horizontal direction of your function is not "
                                "in the expected range of (0, 2*PI]. The value we got was {}",
                                frustum.fov_h));

  // now calculate for xz plane
  is_left_0_p1 =
      is_left(corner_points_x_z_plane[0], corner_points_x_z_plane[1], Point{point.z(), point.x()});
  is_left_0_p2 =
      is_left(corner_points_x_z_plane[0], corner_points_x_z_plane[2], Point{point.z(), point.x()});

  bool is_inside_fov_xz =
      is_inside_fov(frustum.fov_v, is_left_0_p1, is_left_0_p2,
                    fmt::format("The field of view in vertical direction of your function is not "
                                "in the expected range of (0, 2*PI]. The value we got was {}",
                                frustum.fov_v));

  // if we are inside the fovs, we still need to check if the distance is within clip_near and clip_far
  if (is_inside_fov_xy && is_inside_fov_xz) {
    double distance =
        std::sqrt(std::pow(point.x(), 2) + std::pow(point.y(), 2) + std::pow(point.z(), 2));

    if (distance >= frustum.clip_near && distance < frustum.clip_far) {
      ret_val = true;
    }
  }
  return ret_val;
}

}  // namespace cloe::utility
