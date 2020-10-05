#!/usr/bin/env bats
#
# This smoketest requires a VTD installation including the Module Manager
# Open Simulation Interface (OSI) plugin. The plugin is available in the VTD
# wiki and is pre-installed in the Cloe VTD container.

load setup_bats_with_vtd

@test "Run test_vtd_smoketest_osi.json stack file" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    if ! test_vtd_osi_plugin_exists; then
        skip "required osi plugin for simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest_osi.json
    cloe_engine run test_vtd_smoketest_osi.json
}
