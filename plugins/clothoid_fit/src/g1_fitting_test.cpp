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
 * \file g1_fitting_test.cpp
 *
 */

#include <gtest/gtest.h>
#include <vector>

#include "g1_fitting.hpp"  // for calc_clothoid

using namespace g1_fit;  // NOLINT(build/namespaces)

TEST(g1_fitting, fresnel_integral) {
  // Check against results computed with this implementation:
  // https://github.com/ebertolazzi/G1fitting/blob/master/src/Clothoid.cc
  const double tol = 1.0e-8;
  std::vector<double> x{0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 5.0, 7.5, 10.0};
  std::vector<double> c{.00000000, .49234423, .77989340, .44526118, .48825341,
                        .45741301, .56363119, .51601825, .49989869};
  std::vector<double> s{.00000000, .06473243, .43825915, .69750496, .34341568,
                        .61918176, .49919138, .46070123, .46816998};
  for (int i = 0; i < x.size(); ++i) {
    double int_c, int_s;
    calc_std_fresnel_integral(x[i], int_c, int_s);
    EXPECT_NEAR(int_c, c[i], tol);
    EXPECT_NEAR(int_s, s[i], tol);
  }
}
