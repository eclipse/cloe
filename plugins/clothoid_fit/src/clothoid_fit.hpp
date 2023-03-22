/*
 * Copyright 2023 Robert Bosch GmbH
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
 * \file clothoid_fit.hpp
 *
 */

#include <math.h>          // for atan
#include <Eigen/Geometry>  // for Isometry3d
#include <map>             // for map
#include <memory>          // for shared_ptr<>
#include <string>          // for string
#include <vector>          // for vector

#include <cloe/component.hpp>                // for Component, Json
#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/component/lane_sensor.hpp>    // for LaneBoundarySensor
#include <cloe/conf/action.hpp>              // for actions::ConfigureFactory
#include <cloe/handler.hpp>                  // for FromJson, ToJson
#include <cloe/registrar.hpp>                // for Registrar
#include <cloe/sync.hpp>                     // for Sync
#include <cloe/trigger/set_action.hpp>       // for actions::SetVariableActionFactory

#include "g1_fitting.hpp"  // for calc_clothoid

namespace cloe {

struct ClothoidFitConf : public Confable {
  bool enabled = true;
  bool frustum_culling = true;

 public:
  ClothoidFitConf() = default;
  virtual ~ClothoidFitConf() noexcept = default;

  CONFABLE_SCHEMA(ClothoidFitConf) {
    return fable::Schema{
        {"enable", make_schema(&enabled, "enable or disable component")},
        {"frustum_culling", make_schema(&frustum_culling, "enable or disable frustum culling")},
    };
  }
};

namespace component {

/**
 * Estimate lane boundary length by given polyline length.
 *
 * \param points: Lane boundary polyline points.
 */
double estimate_lane_boundary_length(const std::vector<Eigen::Vector3d>& points) {
  double length = 0.0;
  for (uint64_t i = 1; i < points.size(); ++i) {
    length += (points[i] - points[i - 1]).norm();
  }
  return length;
}

void cleanup_lane_boundary_points(std::vector<Eigen::Vector3d>& points) {
  // Points shall have at least 1cm distance for robust clothoid estimation.
  const double min_dist = 0.01;
  std::vector<Eigen::Vector3d> pts_mod{points.front()};
  for (uint64_t i = 1; i < points.size(); ++i) {
    if ((points[i] - points[i - 1]).norm() > min_dist) {
      pts_mod.push_back(points[i]);
    }
  }
  points = pts_mod;
}

// Keep track on which side of each frustum plane a point is located
const uint8_t PLANE_RIGHT_OK{0b0000'0001};
const uint8_t PLANE_LEFT_OK{0b0000'0010};
const uint8_t PLANE_LOW_OK{0b0000'0100};
const uint8_t PLANE_HIGH_OK{0b0000'1000};
const uint8_t PLANE_NEAR_OK{0b0001'0000};
const uint8_t PLANE_FAR_OK{0b0010'0000};
const uint8_t all_set =
    PLANE_RIGHT_OK | PLANE_LEFT_OK | PLANE_LOW_OK | PLANE_HIGH_OK | PLANE_NEAR_OK | PLANE_FAR_OK;

uint8_t get_plane_mask(const Frustum& frustum, const Eigen::Vector3d& pt) {
  double fov_h_l = -0.5 * frustum.fov_h + frustum.offset_h;
  double fov_h_u = 0.5 * frustum.fov_h + frustum.offset_h;
  double fov_v_l = -0.5 * frustum.fov_v + frustum.offset_v;
  double fov_v_u = 0.5 * frustum.fov_v + frustum.offset_v;
  double range_mult = cos(frustum.offset_h) * cos(frustum.offset_v);
  uint8_t plane_mask{0};
  // Horizontal field of view.
  double x = pt.x() == 0.0 ? 1.0e-10 : pt.x();
  double phi = atan2(pt.y(), x);
  if (phi >= fov_h_l) {
    // Right plane.
    plane_mask |= PLANE_RIGHT_OK;
  }
  if (phi <= fov_h_u) {
    // Left plane.
    plane_mask |= PLANE_LEFT_OK;
  }
  // Vertical field of view.
  phi = atan2(pt.z(), x);
  if (phi >= fov_v_l) {
    // Lower plane.
    plane_mask |= PLANE_LOW_OK;
  }
  if (phi <= fov_v_u) {
    // Upper plane.
    plane_mask |= PLANE_HIGH_OK;
  }
  // Field of view distance range.
  x = pt.x() * range_mult;
  if (x >= frustum.clip_near) {
    // Near plane.
    plane_mask |= PLANE_NEAR_OK;
  }
  if (x <= frustum.clip_far) {
    // Far plane.
    plane_mask |= PLANE_FAR_OK;
  }
  return plane_mask;
}

std::vector<Eigen::Vector3d> interpolate_to_frustum_planes(const Frustum& frustum,
                                                           const Eigen::Vector3d& pt1,
                                                           uint8_t mask1,
                                                           const Eigen::Vector3d& pt2,
                                                           uint8_t mask2) {
  auto interpolate_point_on_plane = [](const Eigen::Vector3d& x1, const Eigen::Vector3d& x2,
                                       const Eigen::Vector3d& xp,
                                       const Eigen::Vector3d& nv) -> Eigen::Vector3d {
    // Define line on which the interpolation point is located.
    Eigen::Vector3d dir = x2 - x1;  // direction of interplation line
    double int_loc = nv.dot((xp - x1)) / nv.dot(dir);
    return x1 + int_loc * dir;
  };
  // Shift interpolated points by 1 mm to the inside of the frustum.
  double offset = 0.001;
  std::vector<Eigen::Vector3d> pts_intrp;
  // Use exclusive OR to check for candidates plane by plane.
  if (((mask1 ^ mask2) & PLANE_RIGHT_OK) == PLANE_RIGHT_OK) {
    // Interpolate to right plane.
    double fov_h_l = -0.5 * frustum.fov_h + frustum.offset_h;
    // Normal vector pointing towards the inside of the frustum.
    Eigen::Vector3d nvec =
        Eigen::Vector3d(cos(fov_h_l), sin(fov_h_l), 0).cross(Eigen::Vector3d(0, 0, -1));
    nvec = nvec / nvec.norm();
    Eigen::Vector3d pt = interpolate_point_on_plane(pt1, pt2, Eigen::Vector3d(0, 0, 0), nvec);
    pts_intrp.push_back(pt + offset * nvec);
  }
  if (((mask1 ^ mask2) & PLANE_LEFT_OK) == PLANE_LEFT_OK) {
    // Interpolate to left plane.
    double fov_h_u = 0.5 * frustum.fov_h + frustum.offset_h;
    // Normal vector pointing towards the inside of the frustum.
    Eigen::Vector3d nvec =
        Eigen::Vector3d(cos(fov_h_u), sin(fov_h_u), 0).cross(Eigen::Vector3d(0, 0, 1));
    nvec = nvec / nvec.norm();
    Eigen::Vector3d pt = interpolate_point_on_plane(pt1, pt2, Eigen::Vector3d(0, 0, 0), nvec);
    pts_intrp.push_back(pt + offset * nvec);
  }
  if (((mask1 ^ mask2) & PLANE_LOW_OK) == PLANE_LOW_OK) {
    // Interpolate to lower plane.
    double fov_v_l = -0.5 * frustum.fov_v + frustum.offset_v;
    // Normal vector pointing towards the inside of the frustum.
    Eigen::Vector3d nvec =
        Eigen::Vector3d(cos(fov_v_l), 0, sin(fov_v_l)).cross(Eigen::Vector3d(0, 1, 0));
    nvec = nvec / nvec.norm();
    Eigen::Vector3d pt = interpolate_point_on_plane(pt1, pt2, Eigen::Vector3d(0, 0, 0), nvec);
    pts_intrp.push_back(pt + offset * nvec);
  }
  if (((mask1 ^ mask2) & PLANE_HIGH_OK) == PLANE_HIGH_OK) {
    // Interpolate to higher plane.
    double fov_v_u = 0.5 * frustum.fov_v + frustum.offset_v;
    // Normal vector pointing towards the inside of the frustum.
    Eigen::Vector3d nvec =
        Eigen::Vector3d(cos(fov_v_u), 0, sin(fov_v_u)).cross(Eigen::Vector3d(0, -1, 0));
    nvec = nvec / nvec.norm();
    Eigen::Vector3d pt = interpolate_point_on_plane(pt1, pt2, Eigen::Vector3d(0, 0, 0), nvec);
    pts_intrp.push_back(pt + offset * nvec);
  }
  if (((mask1 ^ mask2) & PLANE_NEAR_OK) == PLANE_NEAR_OK) {
    // Interpolate to near plane.
    double a = cos(frustum.offset_v);
    Eigen::Vector3d nvec = Eigen::Vector3d(a * cos(frustum.offset_h), a * sin(frustum.offset_h),
                                           sin(frustum.offset_v));
    // Normal vector pointing towards the inside of the frustum.
    nvec = nvec / nvec.norm();
    Eigen::Vector3d pt = interpolate_point_on_plane(pt1, pt2, frustum.clip_near * nvec, nvec);
    pts_intrp.push_back(pt + offset * nvec);
  }
  if (((mask1 ^ mask2) & PLANE_FAR_OK) == PLANE_FAR_OK) {
    // Interpolate to far plane.
    double a = cos(frustum.offset_v);
    Eigen::Vector3d nvec = Eigen::Vector3d(a * cos(frustum.offset_h), a * sin(frustum.offset_h),
                                           sin(frustum.offset_v));
    // Normal vector pointing towards the inside of the frustum.
    nvec = -1.0 * nvec / nvec.norm();
    Eigen::Vector3d pt = interpolate_point_on_plane(pt1, pt2, -1.0 * frustum.clip_far * nvec, nvec);
    pts_intrp.push_back(pt + offset * nvec);
  }
  // Remove points that are not on frustum boundary.
  auto it_rm = std::remove_if(
      pts_intrp.begin(), pts_intrp.end(),
      [&frustum](const Eigen::Vector3d& pt) { return get_plane_mask(frustum, pt) != all_set; });
  pts_intrp.erase(it_rm, pts_intrp.end());
  return pts_intrp;
}

// Frustum culling radar approach adapted from
// https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
void lane_boundary_point_culling(const Frustum& frustum, std::vector<Eigen::Vector3d>& points,
                                 std::vector<Eigen::Vector3d>& pts_out_start,
                                 std::vector<Eigen::Vector3d>& pts_out_end) {
  std::vector<uint8_t> mask_out_start, mask_out_end;
  // Keep only points inside the frustum.
  std::vector<Eigen::Vector3d> pts_frustum;
  pts_frustum.reserve(points.size());
  bool found_pts{false};
  for (const auto& pt : points) {
    uint8_t plane_mask = get_plane_mask(frustum, pt);
    // Remove point if not in frustum.
    if (plane_mask == all_set) {
      if (pts_out_end.size() > 0) {
        // In case multiple segments of the polyline are inside the frustum, only keep the first visible segment.
        break;
      }
      if (pts_frustum.size() == 0 && pts_out_start.size() > 0) {
        // First point inside frustum and points outside frustum were found previously. Interpolate to frustum boundary.
        auto pt_intrp = interpolate_to_frustum_planes(frustum, pts_out_start.back(),
                                                      mask_out_start.back(), pt, plane_mask);
        if (pt_intrp.size() == 1) {
          pts_frustum.push_back(pt_intrp.front());
        } else {
          throw cloe::ModelError("Interpolation of first point inside frustum should be unique.");
        }
      }
      pts_frustum.push_back(pt);
      found_pts = true;
    } else if (!found_pts) {
      // Point is outside field of view and no point inside frustum was found previously.
      if (pts_out_start.size() == 0) {
        // First point, no additional checks necessary.
        pts_out_start.push_back(pt);
        mask_out_start.push_back(plane_mask);
      } else {
        // Check special case where the lane boundary crosses the frustum, but none of the given points is located inside.
        Eigen::Vector3d pt_prev = pts_out_start.back();
        // If any of the frustum planes is crossed by the line between previous point and this point, interpolate to frustum boundary.
        auto pts_intrp =
            interpolate_to_frustum_planes(frustum, pt_prev, mask_out_start.back(), pt, plane_mask);
        if (pts_intrp.size() > 0) {
          // If there are multiple points, sort them by distance to the previous point.
          std::sort(pts_intrp.begin(), pts_intrp.end(),
                    [pt_prev](const Eigen::Vector3d& pt1, const Eigen::Vector3d& pt2) {
                      return ((pt1 - pt_prev).norm() < (pt2 - pt_prev).norm());
                    });
          // Append to points inside frustum.
          pts_frustum.insert(pts_frustum.end(), pts_intrp.begin(), pts_intrp.end());
          // This point is outside the frustum towards the end of the lane boundary.
          pts_out_end.push_back(pt);
          mask_out_end.push_back(plane_mask);
        } else {
          // No special case.
          pts_out_start.push_back(pt);
          mask_out_start.push_back(plane_mask);
        }
      }
    } else {
      // Point is outside field of view and at least one point inside was found previously.
      if (pts_out_end.size() == 0) {
        // Interpolate additional point to the frustum boundary.
        auto pt_intrp =
            interpolate_to_frustum_planes(frustum, pts_frustum.back(), all_set, pt, plane_mask);
        if (pt_intrp.size() == 1) {
          pts_frustum.push_back(pt_intrp.front());
        } else {
          throw cloe::ModelError("Interpolation of last point inside frustum should be unique.");
        }
      }
      pts_out_end.push_back(pt);
      mask_out_end.push_back(plane_mask);
    }
  }
  points = pts_frustum;
}

/**
 * Compute heading angle [rad] from two points (line direction from pt0 to pt1).
 *
 * \param pt0: First line point.
 * \param pt1: Second line point.
 */
double calc_heading_angle(const Eigen::Vector3d& pt0, const Eigen::Vector3d& pt1) {
  double dx = pt1.x() - pt0.x();
  dx = dx == 0.0 ? 1.0e-10 : dx;
  double slope = (pt1.y() - pt0.y()) / dx;
  return atan(slope);
}

// https://repository.uantwerpen.be/docman/irua/fb388b/3959.pdf
double calc_heading_angle_adv(const Eigen::Vector3d& ptm1, const Eigen::Vector3d& pt0,
                              const Eigen::Vector3d& pt1) {
  double dx1 = pt1.x() - pt0.x();
  dx1 = dx1 == 0.0 ? 1.0e-10 : dx1;
  double dx0 = pt0.x() - ptm1.x();
  dx0 = dx0 == 0.0 ? 1.0e-10 : dx0;
  double dx = dx1 + dx0;
  dx = dx == 0.0 ? 1.0e-10 : dx;
  double slope = -1.0 * dx1 / (dx0 * dx) * ptm1.y() + (dx1 - dx0) / (dx0 * dx1) * pt0.y() +
                 dx0 / (dx1 * dx) * pt1.y();
  return atan(slope);
}

/**
   * Fit one clothoid segment to the given polyline using a point and heading
   * angle at the beginning and end of the polyline segment of interest,
   * respectively.
   *
   * \param lb: Lane boundary with polyline data.
   */
bool fit_clothoid(const cloe::Logger& logger, bool frustum_culling, const Frustum& frustum,
                  LaneBoundary& lb) {
  if (estimate_lane_boundary_length(lb.points) < 0.5) {
    // Discard tiny lane boundary snippets.
    logger->debug("Discarding short lane boundary segment < 0.5m.");
    return false;
  }
  cleanup_lane_boundary_points(lb.points);
  // Store discarded points for advanced heading angle estimation.
  std::vector<Eigen::Vector3d> pts_out_start, pts_out_end;
  if (frustum_culling) {
    lane_boundary_point_culling(frustum, lb.points, pts_out_start, pts_out_end);
  }
  if (lb.points.size() < 2) {
    logger->debug("Clothoid fit requires at least two points.");
    return false;
  }
  Eigen::Vector3d x0 = lb.points.at(0);
  lb.dx_start = x0.x();
  lb.dy_start = x0.y();
  double hdg0;
  if (pts_out_start.size() > 0) {
    hdg0 = calc_heading_angle_adv(pts_out_start.back(), lb.points.at(0), lb.points.at(1));
  } else {
    hdg0 = calc_heading_angle(lb.points.at(0), lb.points.at(1));
  }
  lb.heading_start = hdg0;
  int n = lb.points.size() - 1;
  Eigen::Vector3d x1 = lb.points.at(n);
  double hdg1;
  if (pts_out_end.size() > 0) {
    hdg1 = calc_heading_angle_adv(lb.points.at(n - 1), lb.points.at(n), pts_out_end.front());
  } else {
    hdg1 = calc_heading_angle(lb.points.at(n - 1), lb.points.at(n));
  }
  // Compute clothoid parameters curv_hor_start, curv_hor_change and dx_end.
  g1_fit::calc_clothoid(x0.x(), x0.y(), hdg0, x1.x(), x1.y(), hdg1, lb.curv_hor_start,
                        lb.curv_hor_change, lb.dx_end);

  return true;
}

class LaneBoundaryClothoidFit : public LaneBoundarySensor {
 public:
  LaneBoundaryClothoidFit(const std::string& name, const ClothoidFitConf& conf,
                          std::shared_ptr<LaneBoundarySensor> sensor)
      : LaneBoundarySensor(name), config_(conf), sensor_in_(sensor) {}
  virtual ~LaneBoundaryClothoidFit() noexcept = default;

  void enroll(Registrar& r) override {
    r.register_action(std::make_unique<actions::ConfigureFactory>(
        &this->config_, "config", "configure clothoid fitting component"));
    r.register_action<actions::SetVariableActionFactory<bool>>(
        "activation", "switch clothoid fitting on/off", "enable", &config_.enabled);
  }

  const LaneBoundaries& sensed_lane_boundaries() const override {
    if (cached_) {
      return lbs_;
    }
    if (config_.enabled) {
      for (auto& kv : sensor_in_->sensed_lane_boundaries()) {
        auto lb = kv.second;
        if (fit_clothoid(logger(), config_.frustum_culling, this->frustum(), lb)) {
          lbs_[kv.first] = lb;
        }
      }
    } else {
      // Copy all lane boundaries from the source sensor component.
      lbs_ = sensor_in_->sensed_lane_boundaries();
    }
    cached_ = true;
    return lbs_;
  }

  const Frustum& frustum() const override { return sensor_in_->frustum(); }

  const Eigen::Isometry3d& mount_pose() const override { return sensor_in_->mount_pose(); }

  /**
   * Process the underlying sensor and clear the cache.
   *
   * We could process and create the filtered list of lane boundaries now, but
   * we can also delay it (lazy computation).
   */
  Duration process(const Sync& sync) override {
    // This currently shouldn't do anything, but this class acts as a prototype
    // for How It Should Be Done.
    Duration t = LaneBoundarySensor::process(sync);
    if (t < sync.time()) {
      return t;
    }

    // Process the underlying sensor and clear the cache.
    t = sensor_in_->process(sync);
    time_ = t;
    clear_cache();
    return t;
  }

  void reset() override {
    LaneBoundarySensor::reset();
    sensor_in_->reset();
    clear_cache();
  }

  void abort() override {
    LaneBoundarySensor::abort();
    sensor_in_->abort();
  }

 protected:
  void clear_cache() {
    lbs_.clear();
    cached_ = false;
  }

 private:
  ClothoidFitConf config_;
  std::shared_ptr<LaneBoundarySensor> sensor_in_;  // provides input data
  mutable bool cached_;
  mutable LaneBoundaries lbs_;
  Duration time_{0};
};

}  // namespace component
}  // namespace cloe
