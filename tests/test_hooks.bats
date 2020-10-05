#!/usr/bin/env bats

load setup_bats

@test "Assert check error on invalid hook" {
    run cloe_engine check test_hook_invalid.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Assert check error on no-exec hook" {
    run cloe_engine check test_hook_noexec.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Assert runtime error on bad hook" {
    cloe_engine check test_hook_failure.json
    run cloe_engine run test_hook_failure.json
    test $status -eq $exit_outcome_aborted
}

@test "Assert success on valid hook" {
    cloe_engine check test_hook_ok.json
    cloe_engine run test_hook_ok.json
}
