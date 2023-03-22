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
 * \file clothoid_test.cpp
 *
 */

#include <gtest/gtest.h>
#include <Eigen/Geometry>  // for Vector3d
#include <cmath>           // for M_PI, M_PI_2

#include <cloe/component/frustum.hpp>        // for Frustum
#include <cloe/component/lane_boundary.hpp>  // for LaneBoundary
#include <cloe/core.hpp>                     // for Json
#include <fable/utility/gtest.hpp>           // for assert_validate
#include "clothoid_fit.hpp"
#include "g1_fitting.hpp"  // for calc_clothoid

using namespace cloe;  // NOLINT(build/namespaces)

TEST(clothoid_fit, deserialization) {
  ClothoidFitConf c;

  fable::assert_validate(c, R"({
      "enable": true,
      "frustum_culling": false
  })");
}

std::vector<Eigen::Vector3d> get_line_lb_points(const Eigen::Vector3d& pt0,
                                                const Eigen::Vector3d& pt1, int n_pts) {
  auto dpt = (pt1 - pt0) / (n_pts - 1);
  std::vector<Eigen::Vector3d> pts(n_pts);
  pts[0] = pt0;
  for (int i = 1; i < n_pts; i++) {
    pts[i] = pts[i - 1] + dpt;
  }
  assert(fabs(pts[n_pts - 1].x() - pt1.x()) < 1E-6);
  return pts;
}

std::vector<Eigen::Vector3d> get_half_circle_lb_points(const Eigen::Vector3d& pt0,
                                                       const Eigen::Vector3d& pt1, int n_pts) {
  double radius = 0.5 * (pt1 - pt0).norm();
  Eigen::Vector3d xc = 0.5 * (pt0 + pt1);
  double phi0 = cloe::component::calc_heading_angle(pt0, pt1);
  // For simplicity, distribute the points equidistantly along x.
  double dphi = M_PI / (n_pts - 1);
  std::vector<Eigen::Vector3d> pts(n_pts);
  for (int i = 0; i < n_pts; i++) {
    // The direction of the circle is clockwise.
    double phi = phi0 + M_PI - i * dphi;
    pts[i] = xc + radius * Eigen::Vector3d(cos(phi), sin(phi), 0);
  }
  const double tol = 1E-6;
  assert(fabs(pts[0].x() - pt0.x()) < tol);
  assert(fabs(pts[0].y() - pt0.y()) < tol);
  assert(fabs(pts[n_pts - 1].x() - pt1.x()) < tol);
  assert(fabs(pts[n_pts - 1].y() - pt1.y()) < tol);
  return pts;
}

TEST(clothoid_fit, circle_calc_curv) {
  const double tol = 1E-6;
  double radius = 10.0;
  Eigen::Vector3d x0 = Eigen::Vector3d(-1.0, 0.0, 0.0);
  Eigen::Vector3d x1 = x0 + 2.0 * Eigen::Vector3d(radius, 0.0, 0.0);
  auto lb_points = get_half_circle_lb_points(x0, x1, 11);
  double k, dk, L;
  g1_fit::calc_clothoid(x0.x(), x0.y(), M_PI_2, x1.x(), x1.y(), -M_PI_2, k, dk, L);
  EXPECT_NEAR(k, -1.0 / radius, tol);
  EXPECT_NEAR(dk, 0.0, tol);
  EXPECT_NEAR(L, radius * M_PI, tol);
}

TEST(clothoid_fit, spiral_calc_curv_change) {
  // Use data from J. Surv. Eng. Vol. 142(3) 04016005, 2016 (doi: 10.1061/(ASCE)SU.1943-5428.0000177)
  const double tol = 1E-4;
  // Refer to Tab. 1 in the paper. Points from the "classical method" are used below.
  double length = 15.0;
  // Note: dk = 1/(aa^2).
  double aa = 17.32;
  double phi0 = 0.0;
  // Refer to Eq. (3) in the paper.
  double phi1 = phi0 + length * length / 2.0 / (aa * aa);
  // First row in Tab. 1.
  Eigen::Vector3d x0 = Eigen::Vector3d(0.0, 0.0, 0.0);
  Eigen::Vector3d x1 = Eigen::Vector3d(14.7904, 1.8563, 0.0);
  double k, dk, L;
  g1_fit::calc_clothoid(x0.x(), x0.y(), phi0, x1.x(), x1.y(), phi1, k, dk, L);
  EXPECT_NEAR(k, 0.0, tol);
  EXPECT_NEAR(dk, 1 / (aa * aa), tol);
  EXPECT_NEAR(L, length, tol);
}

TEST(lane_boundary, clothoid_line) {
  const double tol = 1E-6;
  const double tol_cull = 1.1E-3;  // Interpolation to frustum planes adds up to 1 mm offset.
  auto logger = cloe::logger::get("clothoid");
  cloe::Frustum frustum;
  frustum.fov_h = M_2X_PI;
  frustum.fov_v = M_2X_PI;
  frustum.offset_h = 0.0;
  frustum.offset_v = 0.0;
  frustum.clip_near = -0.001;
  frustum.clip_far = 20.001;
  cloe::LaneBoundary lb;
  // Polyline in x-direction, no culling.
  lb.points =
      get_line_lb_points(Eigen::Vector3d(-5.0, 0.0, 0.0), Eigen::Vector3d(25.0, 0.0, 0.0), 7);
  bool frustum_culling = false;
  ASSERT_TRUE(cloe::component::fit_clothoid(logger, frustum_culling, frustum, lb));
  EXPECT_NEAR(lb.dx_start, -5.0, tol);
  EXPECT_NEAR(lb.dy_start, 0.0, tol);
  EXPECT_NEAR(lb.heading_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_change, 0.0, tol);
  EXPECT_NEAR(lb.dx_end, 30.0, tol);
  // Polyline in x-direction, culling (x-range). Cull at near-plane.
  frustum_culling = true;
  ASSERT_TRUE(cloe::component::fit_clothoid(logger, frustum_culling, frustum, lb));
  EXPECT_NEAR(lb.dx_start, 0.0, tol_cull);
  EXPECT_NEAR(lb.dy_start, 0.0, tol_cull);
  EXPECT_NEAR(lb.heading_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_change, 0.0, tol);
  EXPECT_NEAR(lb.dx_end, 20.0, tol_cull);
  // Polyline in x-direction, culling.
  frustum_culling = true;
  frustum.fov_h = M_PI_2;
  // Cull line at vertical planes.
  frustum.fov_v = M_PI_2;
  // Make sure no culling occurs at near/far planes.
  frustum.clip_near = -10.0;
  frustum.clip_far = 30.0;
  lb.points =
      get_line_lb_points(Eigen::Vector3d(-5.0, 0.0, -5.0), Eigen::Vector3d(25.0, 0.0, -5.0), 7);
  ASSERT_TRUE(cloe::component::fit_clothoid(logger, frustum_culling, frustum, lb));
  EXPECT_NEAR(lb.dx_start, 5.0, tol_cull);
  EXPECT_NEAR(lb.dy_start, 0.0, tol_cull);
  EXPECT_NEAR(lb.heading_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_change, 0.0, tol);
  EXPECT_NEAR(lb.dx_end, 20.0, tol_cull);
  // Polyline in y-direction, no culling.
  frustum_culling = false;
  lb.points =
      get_line_lb_points(Eigen::Vector3d(10.0, -5.0, 0.0), Eigen::Vector3d(10.0, 25.0, 0.0), 7);
  ASSERT_TRUE(cloe::component::fit_clothoid(logger, frustum_culling, frustum, lb));
  EXPECT_NEAR(lb.dx_start, 10.0, tol);
  EXPECT_NEAR(lb.dy_start, -5.0, tol);
  EXPECT_NEAR(lb.heading_start, M_PI_2, tol);
  EXPECT_NEAR(lb.curv_hor_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_change, 0.0, tol);
  EXPECT_NEAR(lb.dx_end, 30.0, tol);
  // Polyline in y-direction, culling.
  frustum_culling = true;
  // Cull at horizontal frustum planes.
  frustum.fov_h = M_PI_2;
  frustum.offset_h = M_PI_4;  // fov = positive quadrant +x/+y or 0..90deg.
  lb.points =
      get_line_lb_points(Eigen::Vector3d(10.0, 0.0, 0.0), Eigen::Vector3d(10.0, 25.0, 0.0), 6);
  ASSERT_TRUE(cloe::component::fit_clothoid(logger, frustum_culling, frustum, lb));
  EXPECT_NEAR(lb.dx_start, 10.0, tol_cull);
  EXPECT_NEAR(lb.dy_start, 0.0, tol_cull);
  EXPECT_NEAR(lb.heading_start, M_PI_2, tol);
  EXPECT_NEAR(lb.curv_hor_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_change, 0.0, tol);
  EXPECT_NEAR(lb.dx_end, 25.0, tol_cull);
  // Polyline in pos. x-, neg. y-direction, no culling.
  frustum_culling = false;
  lb.points =
      get_line_lb_points(Eigen::Vector3d(-5.0, 5.0, 0.0), Eigen::Vector3d(25.0, -25.0, 0.0), 8);
  ASSERT_TRUE(cloe::component::fit_clothoid(logger, frustum_culling, frustum, lb));
  EXPECT_NEAR(lb.dx_start, -5.0, tol);
  EXPECT_NEAR(lb.dy_start, 5.0, tol);
  EXPECT_NEAR(lb.heading_start, -M_PI_4, tol);
  EXPECT_NEAR(lb.curv_hor_start, 0.0, tol);
  EXPECT_NEAR(lb.curv_hor_change, 0.0, tol);
  EXPECT_NEAR(lb.dx_end, (lb.points.back() - lb.points.front()).norm(), tol);
}
