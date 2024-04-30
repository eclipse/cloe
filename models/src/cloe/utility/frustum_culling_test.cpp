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
 * \file cloe/utility/frustum_culling_test.cpp
 * \see  cloe/utility/frustum_culling.hpp
 */

#include <list>
#include <tuple>

#include <gtest/gtest.h>
#include <cloe/utility/frustum_culling.hpp>

TEST(models_frustum_culling, rotate_point) {
  using Point = cloe::utility::Point;
  // Inputs are
  // Point --> point to be rotated
  // double --> angle by which we rotate
  //
  // Output is
  // Point --> expected rotated point
  using input_type = std::tuple<Point, double, Point>;

  // clang-format off
  std::list<input_type> input_list =
  {
    {Point{1.0, 0.0}, M_PI / 4.0,  Point{0.707107, 0.707107}},
    {Point{1.0, 0.0}, M_PI / 2.0,  Point{0.0, 1.0}},
    {Point{1.0, 0.0}, M_PI,        Point{-1.0, 0.0}},
    {Point{1.0, 0.0}, -M_PI / 4.0, Point{0.707107, -0.707107}},
    {Point{1.0, 0.0}, -M_PI / 2.0, Point{0.0, -1.0}},
    {Point{1.0, 0.0}, -M_PI,       Point{-1.0, 0.0}}
  };
  // clang-format on

  int i = 0;
  for (const auto& input : input_list) {
    std::cout << "test case: " << i << std::endl;
    const auto& rotated_point = cloe::utility::rotate_point(std::get<0>(input), std::get<1>(input));
    EXPECT_NEAR(rotated_point.x, std::get<2>(input).x, 0.001);
    EXPECT_NEAR(rotated_point.y, std::get<2>(input).y, 0.001);
    i++;
  }
}

TEST(models_frustum_culling, calc_corner_points) {
  using Point = cloe::utility::Point;
  // inputs are
  // double --> field of view angle
  // double --> offset to input coordinate system
  // double --> clip far
  //
  // output is
  // std::array<double, 3> --> expected output points
  using input_type = std::tuple<double, double, double, std::array<Point, 3>>;
  // clang-format off
  std::list<input_type> input_list =
    {
      {M_PI / 4.0, 0.0,         200.0, {Point{0.0, 0.0}, Point{184.776, -76.537},   Point{184.776, 76.537}}},
      {M_PI / 2.0, 0.0,         300.0, {Point{0.0, 0.0}, Point{212.132, -212.132},  Point{212.132, 212.132}}},
      {1.5 * M_PI, 0.0,         200.0, {Point{0.0, 0.0}, Point{-141.421, -141.421}, Point{-141.421, 141.421}}},
      {M_PI / 2.0, -M_PI / 2.0, 200.0, {Point{0.0, 0.0}, Point{-141.421, -141.421}, Point{141.421, -141.421}}},
      {1.5 * M_PI, -M_PI / 2.0, 200.0, {Point{0.0, 0.0}, Point{-141.421, 141.421},  Point{141.421, 141.421}}},
    };
  // clang-format on

  int i = 0;
  for (const auto& input : input_list) {
    std::cout << "test case: " << i << std::endl;
    const auto& output = cloe::utility::calc_corner_points(std::get<0>(input), std::get<1>(input),
                                                           std::get<2>(input));

    EXPECT_NEAR(output[0].x, std::get<3>(input)[0].x, 0.01);
    EXPECT_NEAR(output[0].y, std::get<3>(input)[0].y, 0.01);
    EXPECT_NEAR(output[1].x, std::get<3>(input)[1].x, 0.01);
    EXPECT_NEAR(output[1].y, std::get<3>(input)[1].y, 0.01);
    EXPECT_NEAR(output[2].x, std::get<3>(input)[2].x, 0.01);
    EXPECT_NEAR(output[2].y, std::get<3>(input)[2].y, 0.01);
    i++;
  }
}

TEST(models_frustum_culling, is_left) {
  using Point = cloe::utility::Point;
  using input_type = std::tuple<Point, Point, Point, bool>;
  // clang-format off
    std::list<input_type> input_list =
    {
      {Point{0.0, 0.0}, Point{1.0, 0.0}, Point{0.0, 0.0},   false},
      {Point{0.0, 0.0}, Point{1.0, 0.0}, Point{0.5, 0.5},   true},
      {Point{0.0, 0.0}, Point{1.0, 0.0}, Point{0.0, 1.0},   true},
      {Point{0.0, 0.0}, Point{1.0, 0.0}, Point{-0.5, 0.5},  true},
      {Point{0.0, 0.0}, Point{1.0, 0.0}, Point{-1.0, 0.0},  false},
      {Point{0.0, 0.0}, Point{1.0, 0.0}, Point{-0.5, -0.5}, false},
      {Point{0.0, 0.0}, Point{1.0, 0.0}, Point{1.0, 0.0},   false},
    };
  // clang-format on
  int i = 0;
  for (const auto& input : input_list) {
    std::cout << "test case: " << i << std::endl;
    const bool output =
        cloe::utility::is_left(std::get<0>(input), std::get<1>(input), std::get<2>(input));

    EXPECT_EQ(output, std::get<3>(input));
    i++;
  }
}

TEST(models_frustum_culling, is_inside_fov) {
  using input_type = std::tuple<double, bool, bool, bool, bool>;
  // clang-format off
  std::list<input_type> input_list = {
    //   fov,      is_left_0_p1, is_left_0_p2, expect_error, result
    {0.0,              true,        false,        true,       true},  // result does not matter as we expect an error
    {M_PI / 2.0,       true,        false,        false,      true},
    {3.0 / 2.0 * M_PI, true,        true,         false,      true},
    {3.0 / 2.0 * M_PI, true,        false,        false,      true},
    {3.0 / 2.0 * M_PI, false,       true,         false,      false},
    {3.0 / 2.0 * M_PI, false,       false,        false,      true},
    {2.0 * M_PI,       true,        true,         false,      true},
    {2.0 * M_PI,       true,        false,        false,      true},
    {2.0 * M_PI,       false,       true,         false,      false},
    {2.0 * M_PI,       false,       false,        false,      true},
    {2.5 * M_PI,       false,       false,        true,       false},
  };
  // clang-format on

  int i = 0;
  for (const auto& input : input_list) {
    std::cout << "test_case: " << i << std::endl;
    // the third input argument gives the information if we expect an error or not
    // if we do not expect an error then compare the result of the function with expected result.
    if (!std::get<3>(input)) {
      bool result = cloe::utility::is_inside_fov(std::get<0>(input), std::get<1>(input),
                                                 std::get<2>(input), "");
      EXPECT_EQ(std::get<4>(input), result);
    } else {
      EXPECT_THROW(cloe::utility::is_inside_fov(std::get<0>(input), std::get<1>(input),
                                                std::get<2>(input), ""),
                   std::runtime_error);
    }
    i++;
  }
}

TEST(models_frustum_culling, is_point_inside_frustum_default) {
  cloe::Frustum frustum{};

  // inputs are point that is passed to the function and expected result
  using input_type = std::tuple<Eigen::Vector3d, bool>;
  std::list<input_type> input_list = {
      {{0.0, 0.0, 0.0}, true},
      {{0.0, 0.0, 1.0}, true},
      {{0.0, 1.0, 0.0}, true},
      {{1.0, 0.0, 0.0}, true},
  };

  int i = 0;
  for (const auto& input : input_list) {
    std::cout << "test case: " << i << std::endl;
    EXPECT_EQ(std::get<1>(input),
              cloe::utility::is_point_inside_frustum(frustum, std::get<0>(input)));
    i++;
  }
}

TEST(models_frustum_culling, is_point_inside_frustum_vary_fov_h) {
  cloe::Frustum frustum{};

  // inputs are frustm fov_h
  using input_type = std::tuple<Eigen::Vector3d, double, bool>;
  // clang-format off
  std::list<input_type> input_list = {
      {{0.01, 0.0, 0.0},   M_PI,       true},
      {{-0.01, 0.0, 0.0},  M_PI,       false},
      {{0.01, 0.0, 0.0},   M_PI,       true},
      {{-0.01, 0.0, 0.0},  M_PI,       false},
      {{0.01, 0.01, 1.0},  M_PI,       true},
      {{0.01, 0.01, -1.0}, M_PI,       true},
      {{1.0, 0.01, 0.0},   M_PI / 2.0, true},
      {{1.0, 100.0, 0.0},  M_PI / 2.0, false},
      {{1.0, -100.0, 0.0}, M_PI / 2.0, false},
      {{-1.0, 100.0, 0.0}, M_PI / 2.0, false},
  };
  // clang-format on

  int i = 0;
  for (const auto& input : input_list) {
    std::cout << "test case: " << i << std::endl;
    frustum.fov_h = std::get<1>(input);
    EXPECT_EQ(std::get<2>(input),
              cloe::utility::is_point_inside_frustum(frustum, std::get<0>(input)));
    i++;
  }
}

TEST(models_frustum_culling, is_point_inside_frustum_vary_offset_h) {
  cloe::Frustum frustum{};
  frustum.fov_h = M_PI;

  using input_type = std::tuple<Eigen::Vector3d, double, bool>;
  // clang-format off
  std::list<input_type> input_list = {
      {{1.0, 0.0, 0.0},  0.0,        true},
      {{-1.0, 0.0, 0.0}, 0.0,        false},
      {{0.0, 1.0, 0.0},  M_PI / 2.0, true},
      {{0.0, -1.0, 0.0}, M_PI / 2.0, false},
      {{1.0, 0.0, 0.0},  M_PI,       false},
      {{-1.0, 0.0, 0.0}, M_PI,       true},
      {{0.0, 1.0, 0.0},  1.5 * M_PI, false},
      {{0.0, -1.0, 0.0}, 1.5 * M_PI, true},
      {{1.0, 0.0, 0.0},  2.0 * M_PI, true},
      {{-1.0, 0.0, 0.0}, 2.0 * M_PI, false},
  };
  // clang-format on

  int i = 0;
  for (const auto& input : input_list) {
    std::cout << "test case: " << i << std::endl;
    frustum.offset_h = std::get<1>(input);
    EXPECT_EQ(std::get<2>(input),
              cloe::utility::is_point_inside_frustum(frustum, std::get<0>(input)));
    i++;
  }
}
