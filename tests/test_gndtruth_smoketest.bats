#!/usr/bin/env bats

load setup_bats_with_vtd

@test "Run test_gndtruth_smoketest.json stack file" {
    if ! test_plugin_exists gndtruth_extractor; then
        skip "required controller gndtruth_extractor not present"
    elif ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi

    local destfile="/tmp/cloe_gndtruth.json.gz"
    if [[ -f $destfile ]]; then
        rm $destfile
    fi
    cloe_engine check test_gndtruth_smoketest.json
    cloe_engine run test_gndtruth_smoketest.json
    test -s $destfile
    rm $destfile
}
