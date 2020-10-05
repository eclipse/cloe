#!/usr/bin/env bats

load setup_bats_with_vtd

@test "Run test_nop_smoketest [ts=5ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_5.json"
    cloe_engine check test_nop_smoketest.json
    cloe_engine run test_nop_smoketest.json
}

@test "Run test_nop_smoketest [ts=60ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_60.json"
    cloe_engine check test_nop_smoketest.json
    cloe_engine run test_nop_smoketest.json
}

@test "Run test_minimator_smoketest [ts=5ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_5.json"
    cloe_engine check test_minimator_smoketest.json
    cloe_engine run test_minimator_smoketest.json
}

@test "Run test_minimator_smoketest [ts=60ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_60.json"
    cloe_engine check test_minimator_smoketest.json
    cloe_engine run test_minimator_smoketest.json
}

@test "Run test_vtd_smoketest [ts=5ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_5.json"
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest.json
    cloe_engine run test_vtd_smoketest.json
}

@test "Run test_vtd_smoketest [ts=60ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_60.json"
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest.json
    cloe_engine run test_vtd_smoketest.json
}
