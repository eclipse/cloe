#!/usr/bin/env bats

load setup_bats

@test "Assert run failure with fail trigger" {
    cloe_engine check test_fail_trigger.json
    run cloe_engine run test_fail_trigger.json
    assert_exit_failure $status
    test $status -eq $exit_outcome_failure
    echo "$output" | grep '"outcome": "failure"'
}
