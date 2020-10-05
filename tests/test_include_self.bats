#!/usr/bin/env bats

load setup_bats

@test "Assert check error with infinite recursion" {
    run cloe_engine check test_include_self.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}
