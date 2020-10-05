#!/usr/bin/env bats

load setup_bats

@test "Assert check error with empty json" {
    run cloe_engine check test_empty.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "Assert check error with no version" {
    run cloe_engine check test_version_absent.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "Assert check error with wrong version" {
    run cloe_engine check test_version_wrong.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
    test $(echo $output 2>&1 | wc -l) -lt 32
}
