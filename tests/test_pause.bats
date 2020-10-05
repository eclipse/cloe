#!/usr/bin/env bats

load setup_bats

@test "Assert pause-resume success" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine check test_pause.json
    cloe_engine run test_pause.json
}
