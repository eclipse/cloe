#!/usr/bin/env bats

load setup_bats

@test "Assert check error on incomplete logging conf" {
    run cloe_engine check test_bad_logging.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}
