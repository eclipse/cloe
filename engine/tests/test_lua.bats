#!/usr/bin/env bats

cd "${BATS_TEST_DIRNAME}"
export CLOE_ROOT="${BATS_TEST_DIRNAME}/../.."
load "${CLOE_ROOT}/tests/setup_bats.bash"
load "${CLOE_ROOT}/tests/setup_testname.bash"

@test "$(testname 'Expect success' 'test_lua01_include_json.lua' '224b2b67-1aaf-4ba2-855c-9bf986574e30')" {
    cloe-engine run test_lua01_include_json.lua
}

@test "$(testname 'Expect success' 'test_lua02_schedule.lua' '93053c17-af8d-461d-b457-e6722e857306')" {
    cloe-engine run --allow-empty test_lua02_schedule.lua
}

@test "$(testname 'Expect success' 'test_lua03_schedule_unpin.lua' '93fe7665-688a-48f5-bd66-4a20a6711ce9')" {
    cloe-engine run test_lua03_schedule_unpin.lua
}

@test "$(testname 'Expect success' 'test_lua04_schedule_test.lua' 'e03fc31f-586b-4e57-80fa-ff2cba5ff9dd')" {
    cloe-engine run test_lua04_schedule_test.lua
}

@test "$(testname 'Expect failure' 'test_lua_error_main.lua' '9cc0c5a4-5771-4cec-befe-ae49bd3e0cae')" {
    run cloe-engine run test_lua_error_main.lua
    assert_check_failure $status $output
    echo "$output" | grep "test_lua_error_main.lua:.*: expect error"
}

@test "$(testname 'Expect failure' 'test_lua_error_coroutine.lua' '9cc0c5a4-5771-4cec-befe-ae49bd3e0cae')" {
    run cloe-engine run test_lua_error_coroutine.lua
    assert_check_failure $status $output
    echo "$output" | grep "test_lua_error_coroutine.lua:.*: expect error"
}

@test "$(testname 'Expect failure' 'test_lua_error_schedule.lua' '9cc0c5a4-5771-4cec-befe-ae49bd3e0cae')" {
    run cloe-engine run test_lua_error_schedule.lua
    assert_check_failure $status $output
    echo "$output" | grep "test_lua_error_schedule.lua:.*: expect error"
}

@test "$(testname 'Expect failure' 'test_lua_error_schedule_test.lua' '9cc0c5a4-5771-4cec-befe-ae49bd3e0cae')" {
    run cloe-engine run test_lua_error_schedule_test.lua
    assert_check_failure $status $output
    echo "$output" | grep "test_lua_error_schedule_test.lua:.*: expect error"
}

@test "$(testname 'Expect segfault' 'test_lua_error_segfault_on_resume.lua' 'df2ac431-7c4d-4253-a38b-f42e0f58a2b2')" {
    run cloe-engine run test_lua_error_segfault_on_resume.lua
    assert_check_failure $status $output
    echo "$output" | grep "segfault"
}
