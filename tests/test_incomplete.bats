#!/usr/bin/env bats

load setup_bats

@test "Assert check error on incomplete conf" {
    run cloe_engine check test_incomplete.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Assert run error on incomplete conf" {
    run cloe_engine run test_incomplete.json
    test $status -eq $exit_outcome_unknown
}
