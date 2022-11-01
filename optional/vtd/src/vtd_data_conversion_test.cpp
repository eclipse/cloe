/*
 * Copyright 2021 Robert Bosch GmbH

 */
/**
 * \file vtd_data_conversion_test.cpp
 * \see  omni_sensor_component.hpp
 * \see  task_control.hpp
 */

#include <cmath>  // for M_PI

#include <gtest/gtest.h>              // for TEST, ASSERT_TRUE
#include <cloe/component/object.hpp>  // for Object

#include "omni_sensor_component.hpp"
#include "task_control.hpp"

RDB_COORD_t get_test_rdb_coord() {
  RDB_COORD_t c;
  c.x = 1.0;
  c.y = 2.0;
  c.z = 3.0;
  c.h = 0.1 * M_PI;
  c.p = 0.2 * M_PI;
  c.r = 0.3 * M_PI;
  c.flags = RDB_COORD_FLAG_POINT_VALID | RDB_COORD_FLAG_ANGLES_VALID;
  return c;
}

void assert_rdb_coord_eq(const RDB_COORD_t& c1, const RDB_COORD_t& c2) {
  ASSERT_DOUBLE_EQ(c1.x, c2.x);
  ASSERT_DOUBLE_EQ(c1.y, c2.y);
  ASSERT_DOUBLE_EQ(c1.z, c2.z);
  if (c2.flags & RDB_COORD_FLAG_ANGLES_VALID) {
    ASSERT_DOUBLE_EQ(c1.h, c2.h);
    ASSERT_DOUBLE_EQ(c1.p, c2.p);
    ASSERT_DOUBLE_EQ(c1.r, c2.r);
  }
}

TEST(vtd_data, rdb_coord) {
  // Convert from VTD to Cloe data and back.
  RDB_COORD_t coord = get_test_rdb_coord();
  cloe::Object obj;
  obj.pose = vtd::from_vtd_pose(coord);
  RDB_COORD_t coord2 = vtd::rdb_coord_from_object(obj);
  assert_rdb_coord_eq(coord, coord2);
  coord2 = vtd::rdb_coord_pos_from_vector3d(obj.pose.translation());
  assert_rdb_coord_eq(coord, coord2);
}
