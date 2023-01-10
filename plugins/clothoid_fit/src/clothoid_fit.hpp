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

  double estimation_dist;

 public:
  ClothoidFitConf() = default;
  virtual ~ClothoidFitConf() noexcept = default;

  CONFABLE_SCHEMA(ClothoidFitConf) {
    return fable::Schema{
        {"enable", make_schema(&enabled, "enable or disable component")},
        {"frustum_culling", make_schema(&frustum_culling, "enable or disable frustum culling")},

        {"estimation_distance",
         make_schema(&estimation_dist,
                     "consider polyline points within this distance for clothoid estimation")
             .require()},
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
 * Interpolate point along line onto x-normal plane located at x_loc.
 *
 * \param x_l: Line point.
 * \param x_loc: x-coordinate of the target x-normal plane.
 * \param x_approx: On entry: approximate location of interest; on exit: crossing of the given line and x-normal plane,
 * if the line crosses the plane at least at an angle of 45 deg.
 */
void interpolate_point_x_direction(const Eigen::Vector3d& x_l, double x_loc,
                                   Eigen::Vector3d& x_approx) {
  // Define x-normal plane located at x_loc.
  Eigen::Vector3d z(x_loc, 0, 0);  // point on the plane
  Eigen::Vector3d n_vec(1, 0, 0);  // plane normal vector
  // Define line on which the interpolation point is located.
  Eigen::Vector3d dir = x_approx - x_l;  // direction of interplation line
  if (fabs(n_vec.dot(dir)) > 0.7071) {
    // Only interpolate if the line crosses the plane in > 45deg angle. Keep x_approx otherwise.
    double int_loc = n_vec.dot((z - x_l)) / n_vec.dot(dir);
    x_approx = x_l + int_loc * dir;
  }
}

/**
 * Interpolate clothoid start point and heading angle in given x-distance.
 *
 * \param points: Lane boundary polyline points.
 * \param min_dist_hdg_est: Distance between two points for heading angle estimation.
 * \param x_loc: x-coordinate of clothoid start point.
 * \param x0: Clothoid start point.
 * \param hdg0: Heading angle at clothoid start point [rad].
 */
void get_clothoid_point_heading_start(const std::vector<Eigen::Vector3d>& points,
                                      double min_dist_hdg_est, double x_loc, Eigen::Vector3d& x0,
                                      double& hdg0) {
  // Assume that the first polyline points are <= x_loc TODO(tobias).
  auto x0_it = std::find_if(points.begin(), points.end(),
                            [&](const Eigen::Vector3d& pt) { return pt.x() >= x_loc; });
  if (x0_it != points.end()) {
    // Determine heading angle.
    x0 = *x0_it;  // only temporary value, will be interpolated to x_loc below.
    // Try to approximate tangent by marching forward along polyline.
    auto x0p1_it = ++x0_it;  // x0p1_it lies further down along the polyline than x0_it.
    while (x0p1_it != points.end() && (*x0p1_it - x0).norm() < min_dist_hdg_est) {
      ++x0p1_it;
    }
    if (x0p1_it == points.end()) {
      // Try to approximate tangent by marching backward along polyline.
      x0p1_it = x0_it;
      while (x0_it != points.begin() && (x0 - *x0_it).norm() < min_dist_hdg_est) {
        --x0_it;
      }
      if ((x0 - *x0_it).norm() < min_dist_hdg_est) {
        throw cloe::ModelError("Point for heading estimation at x0 was not found.");
      }
    }
    hdg0 = calc_heading_angle(x0, *x0p1_it);
    // Interpolate point to a plane normal to the x-direction.
    interpolate_point_x_direction(*x0p1_it, x_loc, x0);
  } else {
    throw cloe::ModelError("Point x0 not found.");
  }
}

void get_clothoid_point_heading_end(const std::vector<Eigen::Vector3d>& points,
                                    const Eigen::Vector3d& x0, double min_dist_hdg_est, double dist,
                                    Eigen::Vector3d& x1, double& hdg1) {
  auto x0_it = std::find_if(points.begin(), points.end(),
                            [&](const Eigen::Vector3d& pt) { return pt.x() >= x0.x(); });
  auto x1_it = std::find_if(x0_it, points.end(),
                            [&](const Eigen::Vector3d& pt) { return (pt - x0).norm() >= dist; });
  if (x1_it == points.end()) {
    --x1_it;
  }
  x1 = *x1_it;
  // Determine heading angle.
  auto it = --x1_it;
  while ((x1 - *it).norm() < min_dist_hdg_est && it != points.begin()) {
    --it;
  }
  if ((x1 - *it).norm() < min_dist_hdg_est) {
    throw cloe::ModelError("Point for heading estimation at x1 was not found.");
  }
  hdg1 = calc_heading_angle(*it, x1);
  // Interpolate point to a plane normal to the x-direction.
  interpolate_point_x_direction(*it, x0.x() + dist, x1);
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
        if (fit_clothoid(lb)) {
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
  // Frustum culling radar approach ToDo(tobias) https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
  void lane_boundary_point_culling(std::vector<Eigen::Vector3d>& points,
                                   std::vector<Eigen::Vector3d>& pts_out_start,
                                   std::vector<Eigen::Vector3d>& pts_out_end) const {
    double fov_h_l = -0.5 * this->frustum().fov_h + this->frustum().offset_h;
    double fov_h_u = 0.5 * this->frustum().fov_h + this->frustum().offset_h;
    double fov_v_l = -0.5 * this->frustum().fov_v + this->frustum().offset_v;
    double fov_v_u = 0.5 * this->frustum().fov_v + this->frustum().offset_v;
    double range_mult = cos(this->frustum().offset_h) * cos(this->frustum().offset_v);
    // Keep track on which side of each frustum plane a point is located
    const uint8_t PLANE_RIGHT_OK{0b0000'0001};
    const uint8_t PLANE_LEFT_OK{0b0000'0010};
    const uint8_t PLANE_LOW_OK{0b0000'0100};
    const uint8_t PLANE_HIGH_OK{0b0000'1000};
    const uint8_t PLANE_NEAR_OK{0b0001'0000};
    const uint8_t PLANE_FAR_OK{0b0010'0000};
    const uint8_t all_set = PLANE_RIGHT_OK | PLANE_LEFT_OK | PLANE_LOW_OK | PLANE_HIGH_OK |
                            PLANE_NEAR_OK | PLANE_FAR_OK;
    // Keep only points inside the frustum.
    std::vector<Eigen::Vector3d> pts_frustum;
    pts_frustum.reserve(points.size());
    bool found_pts{false};
    for (const auto& pt : points) {
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
      if (x >= this->frustum().clip_near) {
        // Near plane.
        plane_mask |= PLANE_NEAR_OK;
      }
      if (x <= this->frustum().clip_far) {
        // Far plane.
        plane_mask |= PLANE_FAR_OK;
      }
      // Remove point if not in frustum.
      if (plane_mask == all_set) {
        if (pts_out_end.size() > 0) {
          // In case multiple segments of the polyline are inside the frustum, only keep the first visible segment.
          break;
        }
        pts_frustum.push_back(pt);
        found_pts = true;
      } else if (!found_pts) {
        // Point is outside field of view and no point inside was found previously.
        pts_out_start.push_back(pt);
      } else {
        // Point is outside field of view and at least one point inside was found previously.
        pts_out_end.push_back(pt);
      }
    }
    points = pts_frustum;
  }

  /**
   * Fit one clothoid segment to the given polyline using a point and heading
   * angle at the beginning (x=0) and end (=estimation_distance) of the polyline
   * segment of interest, respectively.
   *
   * \param lb: Lane boundary with polyline data.
   */
  bool fit_clothoid(LaneBoundary& lb) const {
    if (estimate_lane_boundary_length(lb.points) < 0.5) {
      // Discard tiny lane boundary snippets.
      logger()->debug("Discarding short lane boundary segment < 0.5m.");
      return false;
    }
    cleanup_lane_boundary_points(lb.points);
    // Store discarded points for advanced heading angle estimation.
    std::vector<Eigen::Vector3d> pts_out_start, pts_out_end;
    if (config_.frustum_culling) {
      lane_boundary_point_culling(lb.points, pts_out_start, pts_out_end);
    }
    if (lb.points.size() < 2) {
      logger()->debug("Clothoid fit requires at least two points.");
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

    // TODO(tobias): check zero crossing by loop
    /*if (lb.points.front().x() <= 0.0 && lb.points.back().x() >= 0.0) {
      // Determine start point.
      Eigen::Vector3d x0;
      double hdg0 = 0.0;
      // Require at least 1m distance between points for heading estimation.
      double min_dist_hdg_est = 1.0;
      get_clothoid_point_heading_start(lb.points, min_dist_hdg_est, 0.0, x0, hdg0);
      lb.dx_start = x0.x();
      lb.dy_start = x0.y();
      lb.heading_start = hdg0;
      // Determine end point.
      Eigen::Vector3d x1;
      double hdg1 = 0.0;
      get_clothoid_point_heading_end(lb.points, x0, min_dist_hdg_est, config_.estimation_dist, x1,
                                     hdg1);
      // Compute clothoid parameters curv_hor_start, curv_hor_change and dx_end.
      g1_fit::calc_clothoid(x0.x(), x0.y(), hdg0, x1.x(), x1.y(), hdg1, lb.curv_hor_start,
                            lb.curv_hor_change, lb.dx_end);
    } else {
      logger()->debug(
          "Lane boundary {} does not have a zero crossing in x-direction. Skipping clothoid "
          "calculation..",
          lb.id);
    }*/
    return true;
  }

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
