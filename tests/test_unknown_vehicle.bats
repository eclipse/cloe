#!/usr/bin/env bats

load setup_bats

@test "Assert check error with unknown vehicle" {
    run cloe_engine check test_unknown_vehicle.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Assert run error with unknown vehicle" {
    run cloe_engine run test_unknown_vehicle.json
    test $status -eq $exit_outcome_unknown
}

