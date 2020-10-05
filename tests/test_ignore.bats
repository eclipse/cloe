#!/usr/bin/env bats

load setup_bats

@test "Assert run success with ignored sections" {
    cloe_engine check test_ignore.json
    cloe_engine run test_ignore.json
}
