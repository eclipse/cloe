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
 * \file cloe/utility/statistics_test.cpp
 * \see  cloe/utility/statistics.hpp
 */

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
namespace ba = boost::accumulators;
namespace tag = boost::accumulators::tag;

#include <cloe/utility/statistics.hpp>
using cloe::utility::Accumulator;
using cloe::utility::Pie;

TEST(utility_statistics_pie, with_int) {
  std::vector<int> data{1, 1, 1, 1, 2, 2, 3, 4, 1, 2, 0, 3, 2};

  Pie<int> pie;
  for (auto x : data) {
    pie.push_back(x);
  }

  EXPECT_EQ(1, pie.mode());
  EXPECT_EQ(data.size(), pie.count());
  EXPECT_EQ(static_cast<uint64_t>(2), pie.count(3));
  EXPECT_EQ(static_cast<uint64_t>(4), pie.count(2));
  EXPECT_EQ(static_cast<double>(5) / static_cast<double>(data.size()), pie.proportion(1));
}

TEST(utility_statistics_accumulator, with_double) {
  std::vector<double> data{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};

  Accumulator my_acc;
  ba::accumulator_set<
      double, ba::stats<tag::mean, tag::count, tag::max, tag::min, tag::variance, tag::median>>
      ref_acc;
  for (auto x : data) {
    my_acc.push_back(x);
    ref_acc(x);
  }

  EXPECT_EQ(ba::count(ref_acc), my_acc.count());
  EXPECT_EQ(ba::min(ref_acc), my_acc.min());
  EXPECT_EQ(ba::max(ref_acc), my_acc.max());  // NOLINT
  EXPECT_EQ(ba::mean(ref_acc), my_acc.mean());
  EXPECT_EQ(ba::variance(ref_acc), my_acc.variance());
}
