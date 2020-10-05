#!/usr/bin/env bats

load setup_bats_with_vtd

@test "Run test_clean_timeout.json stack file" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi

    run cloe_engine run test_clean_timeout.json
    assert_check_failure $status $output
    # This will look weird, that because of BATS.
    echo $output
    test $status -eq $exit_outcome_unknown
    echo $output | grep '### test successful ###'
}
