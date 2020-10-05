#!/usr/bin/env bats

load setup_bats_with_vtd

@test "Assert watchdog success" {
    if ! type curl &>/dev/null; then
        skip "required program curl not present"
    fi

    cloe_engine check test_watchdog.json

    # assert abort with core dump, code 134/250
    run cloe_engine run test_watchdog.json
    test $status -eq $exit_outcome_syskill
}

@test "Assert vtd watchdog success" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator VTD not present"
    fi
    if ! type killall &>/dev/null; then
        skip "required program killall not present"
    fi

    cloe_engine check test_vtd_watchdog.json

    # assert abort with core dump, code 134/250
    run cloe_engine run test_vtd_watchdog.json
    test $status -eq $exit_outcome_syskill
}

@test "Assert vtd watchdog success" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator VTD not present"
    fi
    cloe_engine check test_vtd_watchdog.json
    ! cloe_engine run test_vtd_watchdog.json
}
