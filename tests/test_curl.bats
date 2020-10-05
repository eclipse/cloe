#!/usr/bin/env bats

load setup_bats

@test "Assert run success with curl trigger" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine check test_curl_succeed.json
    cloe_engine run test_curl_succeed.json
}

@test "Assert run success with commands disable" {
    run cloe_engine run test_curl_succeed.json \
        <(echo '{ "version": "4", "engine": { "security": { "enable_command_action": false } } }')
    test $status -eq $exit_outcome_failure
}
