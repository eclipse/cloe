#!/usr/bin/env bats

load setup_bats

@test "Run test_nop_smoketest.json stack file" {
    cloe_engine check test_nop_smoketest.json
    cloe_engine run test_nop_smoketest.json
}
