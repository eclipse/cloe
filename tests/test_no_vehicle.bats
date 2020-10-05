#!/usr/bin/env bats

load setup_bats

@test "Assert check error with no vehicle" {
    run cloe_engine check test_no_vehicle.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Assert run error with no vehicle" {
    run cloe_engine run test_no_vehicle.json
    test $status -eq $exit_outcome_unknown
}
