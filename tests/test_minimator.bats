#!/usr/bin/env bats

load setup_bats

@test "Expect check success    : test_minimator_smoketest.json" {
    cloe_engine check test_minimator_smoketest.json
}

@test "Expect run success      : test_minimator_smoketest.json" {
    cloe_engine run test_minimator_smoketest.json
}

@test "Expect check/run success: test_minimator_smoketest.json [ts=5ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_5.json"
    cloe_engine check test_minimator_smoketest.json
    cloe_engine run test_minimator_smoketest.json
}

@test "Expect check/run success: test_minimator_smoketest.json [ts=60ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_60.json"
    cloe_engine check test_minimator_smoketest.json
    cloe_engine run test_minimator_smoketest.json
}

@test "Expect check/run success: test_minimator_multi_agent_smoketest.json" {
    cloe_engine check test_minimator_multi_agent_smoketest.json
    cloe_engine run test_minimator_multi_agent_smoketest.json
}
