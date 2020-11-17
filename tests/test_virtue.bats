#!/usr/bin/env bats

load setup_bats

@test "Expect check success    : test_virtue_missing_lanes_fail.json" {
    cloe_engine check test_virtue_missing_lanes_fail.json
}

# shall fail because nop simulator provides no lanes
@test "Expect run failure      : test_virtue_missing_lanes_fail.json" {
    ! cloe_engine run test_virtue_missing_lanes_fail.json
}

@test "Expect check success    : test_virtue_missing_lanes_pass.json" {
    cloe_engine check test_virtue_missing_lanes_pass.json
}

# shall pass because minimator provides lanes
@test "Expect run success      : test_virtue_missing_lanes_pass.json" {
    cloe_engine run test_virtue_missing_lanes_pass.json
}
