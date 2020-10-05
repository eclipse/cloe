#!/usr/bin/env bats

load setup_bats

@test "Run test_minimator_smoketest.json stack file" {
    cloe_engine check test_minimator_smoketest.json
    cloe_engine run test_minimator_smoketest.json
}
