#!/usr/bin/env bats

cd "${BATS_TEST_DIRNAME}"
export CLOE_ROOT="${BATS_TEST_DIRNAME}/../.."
load "${CLOE_ROOT}/tests/setup_bats.bash"
load "${CLOE_ROOT}/tests/setup_testname.bash"

@test "$(testname 'Expect success' 'test_lua01_include_json.lua' '224b2b67-1aaf-4ba2-855c-9bf986574e30')" {
    cloe-engine check test_lua01_include_json.lua
    cloe-engine run test_lua01_include_json.lua
}

@test "$(testname 'Expect success' 'test_lua02_schedule.lua' '93053c17-af8d-461d-b457-e6722e857306')" {
    cloe-engine check test_lua02_schedule.lua
    cloe-engine run test_lua02_schedule.lua
}
