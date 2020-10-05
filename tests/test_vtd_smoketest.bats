#!/usr/bin/env bats

load setup_bats_with_vtd

@test "Run test_vtd_smoketest.json stack file" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest.json
    cloe_engine run test_vtd_smoketest.json
}
