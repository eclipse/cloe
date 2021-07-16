#!/usr/bin/env bats

load setup_bats_with_sumo

export CLOE_PROFILE_FILE="conanfile.txt"

@test "Run test_sumo_basic.json stack file" {
    if ! test_sumo_plugin_exists; then
        skip "required simulator sumo not present"
    fi
    cloe-engine check test_sumo_basic.json
    cloe-engine run test_sumo_basic.json
}
