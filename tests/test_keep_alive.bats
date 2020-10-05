#!/usr/bin/env bats

load setup_bats

@test "Assert termination after keep-alive" {
    if ! type kill &>/dev/null; then
        skip "required program kill not present"
    fi

    cloe_engine check test_keep_alive.json
    cloe_engine run test_keep_alive.json
}
