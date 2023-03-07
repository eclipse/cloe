#!/usr/bin/env bats

cd "${BATS_TEST_DIRNAME}"
export CLOE_ROOT="${BATS_TEST_DIRNAME}/../../.."
load "${CLOE_ROOT}/tests/setup_bats.bash"
load "${CLOE_ROOT}/tests/setup_testname.bash"

@test "$(testname 'Expect run failure' 'test_gndtruth_invalid_file.json' '29e8ff3b-86f3-4320-ba69-05600df1f836')" {
    cloe-engine check test_gndtruth_invalid_file.json
    run cloe-engine run test_gndtruth_invalid_file.json
    assert_exit_failure $status
    test $status -eq $CLOE_EXIT_ABORTED
}
