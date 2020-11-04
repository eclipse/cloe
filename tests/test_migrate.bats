#!/usr/bin/env bats

load setup_bats

@test "Check: test_migrate_v3.json" {
    cloe_engine check test_migrate_v3.json
}

@test "Run:   test_migrate_v3.json" {
    cloe_engine run test_migrate_v3.json
}
