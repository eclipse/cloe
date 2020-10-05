#!/usr/bin/env bats

load setup_bats_with_vtd

@test "Assert run error on unknown sensor reference" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi

    run cloe_engine run test_vtd_unknown_sensor.json
    test $status -eq $exit_outcome_unknown
}
