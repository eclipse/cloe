#!/usr/bin/env bats

load setup_bats_with_vtd

@test "Run test_minimator_multi_agent_smoketest" {
    cloe_engine check test_minimator_multi_agent_smoketest.json
    cloe_engine run test_minimator_multi_agent_smoketest.json
}

# Note our open support request regarding stimulating multiple external
# vehicles in VTD: https://redmine.vires.com/issues/13340
#
# TODO: Improve the tested condition once we now how to correctly deal with VTD
@test "Run test_vtd_multi_agent_smoketest" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_multi_agent_smoketest.json
    cloe_engine run test_vtd_multi_agent_smoketest.json
}
