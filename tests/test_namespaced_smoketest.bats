#!/usr/bin/env bats

load setup_bats

@test "Run test_namespaced_smoketest.json stack file" {
    cloe_engine check test_namespaced_smoketest.json
    cloe_engine run test_namespaced_smoketest.json
}
