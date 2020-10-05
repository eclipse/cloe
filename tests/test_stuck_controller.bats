#!/usr/bin/env bats

load setup_bats

@test "Assert run failure with stuck controller" {
    cloe_engine check test_stuck_controller.json
    run cloe_engine run test_stuck_controller.json
    assert_exit_failure $status
    echo "$output" | grep '"outcome": "aborted"'
}

@test "Assert run success with stuck controller ignored" {
    cloe_engine check test_stuck_controller_continue.json
    cloe_engine run test_stuck_controller_continue.json
}
