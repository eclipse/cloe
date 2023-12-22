#!/usr/bin/env bats

cd "${BATS_TEST_DIRNAME}"
export CLOE_ROOT="${BATS_TEST_DIRNAME}/../../.."
load "${CLOE_ROOT}/tests/setup_bats.bash"
load "${CLOE_ROOT}/tests/setup_testname.bash"

@test "$(testname 'Expect check success' 'test_esmini_open_loop.json' '05be11dc-1904-4f5e-bed7-531adf51a55c')" {
    cloe-engine check test_esmini_open_loop.json
}

@test "$(testname 'Expect run success' 'test_esmini_open_loop.json' 'bb3ee099-f294-4e8b-922b-2cfc7adf03df')" {
    cloe-engine run test_esmini_open_loop.json
}

@test "$(testname 'Expect check success' 'test_esmini_closed_loop.json' '50c9dadf-da81-4f91-828a-ceee33570eae')" {
    cloe-engine check test_esmini_closed_loop.json
}

@test "$(testname 'Expect run success' 'test_esmini_closed_loop.json' '2f34d19c-906a-458b-97ec-b0a1b6d0e294')" {
    cloe-engine run test_esmini_closed_loop.json
}
