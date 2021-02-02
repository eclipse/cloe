#!/usr/bin/env bats

load setup_bats

test_vtd_plugin_exists() {
    # VTD_ROOT is only available in the shell.
    cloe_shell -c 'test -d "${VTD_ROOT}"'
}

@test "Expect schema equality  : test_engine_json_schema.json" {
    if ! type diff &>/dev/null; then
        skip "required program diff not present"
    fi

    if test_vtd_plugin_exists; then
        diff <(cloe_engine usage -j 2>/dev/null) test_engine_json_schema_with_vtd.json
    else
        diff <(cloe_engine usage -j 2>/dev/null) test_engine_json_schema.json
    fi
}

@test "Expect check success    : test_engine_smoketest.json" {
    cloe_engine check test_engine_smoketest.json
}

@test "Expect run success      : test_engine_smoketest.json" {
    cloe_engine run test_engine_smoketest.json
}

@test "Expect check failure    : test_engine_bad_logging.json" {
    run cloe_engine check test_bad_logging.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect check success    : test_engine_curl_succeed.json" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine check test_engine_curl_succeed.json
}

@test "Expect run success      : test_engine_curl_succeed.json" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi
    cloe_engine run test_engine_curl_succeed.json
}

@test "Expect run failure      : test_engine_curl_succeed.json" {
    # When disabling the `enable_command_action` flag, curl
    # should not be used and the simulation should fail by default.
    run cloe_engine run test_engine_curl_succeed.json \
        <(echo '{ "version": "4", "engine": { "security": { "enable_command_action": false } } }')
    test $status -eq $exit_outcome_failure
}

@test "Expect check failure    : test_engine_empty.json" {
    run cloe_engine check test_engine_empty.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "Expect check failure    : test_engine_version_absent.json" {
    run cloe_engine check test_engine_version_absent.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "Expect check failure    : test_engine_version_wrong.json" {
    run cloe_engine check test_engine_version_wrong.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "Expect check success    : test_engine_fail_trigger.json" {
    cloe_engine check test_engine_fail_trigger.json
}

@test "Expect run failure      : test_engine_fail_trigger.json" {
    run cloe_engine run test_engine_fail_trigger.json
    assert_exit_failure $status
    test $status -eq $exit_outcome_failure
    echo "$output" | grep '"outcome": "failure"'
}

@test "Expect check failure    : test_engine_hook_invalid.json" {
    run cloe_engine check test_engine_hook_invalid.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect check failure    : test_engine_hook_noexec.json" {
    run cloe_engine check test_engine_hook_noexec.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect check success    : test_engine_hook_failure.json" {
    cloe_engine check test_engine_hook_failure.json
}

@test "Expect run failure      : test_engine_hook_failure.json" {
    run cloe_engine run test_engine_hook_failure.json
    test $status -eq $exit_outcome_aborted
}

@test "Expect check success    : test_engine_hook_ok.json" {
    cloe_engine check test_engine_hook_ok.json
}

@test "Expect run success      : test_engine_hook_ok.json" {
    cloe_engine run test_engine_hook_ok.json
}

@test "Expect check success    : test_engine_ignore.json" {
    cloe_engine check test_engine_ignore.json
}

@test "Expect run success      : test_engine_ignore.json" {
    cloe_engine run test_engine_ignore.json
}

@test "Expect check failure    : test_engine_include_nonexistent.json" {
    run cloe_engine check test_engine_include_nonexistent.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect check failure    : test_engine_include_self.json" {
    run cloe_engine check test_engine_include_self.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect check failure    : test_engine_incomplete.json" {
    run cloe_engine check test_engine_incomplete.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect run failure      : test_engine_incomplete.json" {
    run cloe_engine run test_engine_incomplete.json
    test $status -eq $exit_outcome_unknown
}

@test "Expect check success    : test_engine_keep_alive.json" {
    if ! type kill &>/dev/null; then
        skip "required program kill not present"
    fi

    cloe_engine check test_engine_keep_alive.json
}

@test "Expect check success    : test_engine_keep_alive.json" {
    if ! type kill &>/dev/null; then
        skip "required program kill not present"
    fi

    cloe_engine run test_engine_keep_alive.json
}

@test "Expect check success    : test_engine_namespaced_smoketest.json" {
    cloe_engine check test_engine_namespaced_smoketest.json
}

@test "Expect run success      : test_engine_namespaced_smoketest.json" {
    cloe_engine run test_engine_namespaced_smoketest.json
}

@test "Expect check failure    : test_engine_no_binding.json" {
    run cloe_engine check test_engine_no_binding.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect run failure      : test_engine_no_binding.json" {
    run cloe_engine run test_engine_no_binding.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect check failure    : test_engine_no_vehicle.json" {
    run cloe_engine check test_engine_no_vehicle.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect run failure      : test_engine_no_vehicle.json" {
    run cloe_engine run test_engine_no_vehicle.json
    test $status -eq $exit_outcome_unknown
}

@test "Expect check failure    : test_engine_unknown_vehicle.json" {
    run cloe_engine check test_engine_unknown_vehicle.json
    assert_check_failure $status $output
    test $status -eq $exit_outcome_unknown
}

@test "Expect run failure      : test_engine_unknown_vehicle.json" {
    run cloe_engine run test_engine_unknown_vehicle.json
    test $status -eq $exit_outcome_unknown
}

@test "Expect check success    : test_engine_pause.json" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine check test_engine_pause.json
}

@test "Expect run success      : test_engine_pause.json" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine run test_engine_pause.json
}

@test "Expect check success    : test_engine_sticky_trigger.json" {
    cloe_engine check test_engine_sticky_trigger.json
}

@test "Expect run success      : test_engine_sticky_trigger.json" {
    skip "not implemented yet"
    cloe_engine run test_engine_sticky_trigger.json
}

@test "Expect check success    : test_engine_stuck_controller.json" {
    cloe_engine check test_engine_stuck_controller.json
}

@test "Expect run aborted      : test_engine_stuck_controller.json" {
    run cloe_engine run test_engine_stuck_controller.json
    assert_exit_failure $status
    echo "$output" | grep '"outcome": "aborted"'
}

@test "Expect check success    : test_engine_stuck_controller_continue.json" {
    cloe_engine check test_engine_stuck_controller_continue.json
}

@test "Expect run success      : test_engine_stuck_controller_continue.json" {
    cloe_engine run test_engine_stuck_controller_continue.json
}

@test "Expect check success    : test_engine_watchdog.json" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine check test_engine_watchdog.json
}

@test "Expect run syskill      : test_engine_watchdog.json" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine check test_engine_watchdog.json

    # assert abort with core dump, code 134/250
    run cloe_engine run test_engine_watchdog.json
    test $status -eq $exit_outcome_syskill
}

@test "Expect check/run success: test_engine_smoketest.json [ts=5ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_5.json"
    cloe_engine check test_engine_smoketest.json
    cloe_engine run test_engine_smoketest.json
}

@test "Expect check/run success: test_engine_smoketest.json [ts=60ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_60.json"
    cloe_engine check test_engine_smoketest.json
    cloe_engine run test_engine_smoketest.json
}
