#!/usr/bin/env bats

load setup_bats

# shall fail because nop simulator provides no lanes
@test "Run test_missing_lanes_fail.json stack file" {
    cloe_engine check test_missing_lanes_fail.json
    ! cloe_engine run test_missing_lanes_fail.json
}

# shall pass because minimator provides lanes
@test "Run test_missing_lanes_pass.json stack file" {
    cloe_engine check test_missing_lanes_pass.json
    cloe_engine run test_missing_lanes_pass.json
}
