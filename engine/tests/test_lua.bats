#!/usr/bin/env bats

cd "${BATS_TEST_DIRNAME}"
export CLOE_ROOT="${BATS_TEST_DIRNAME}/../.."
load "${CLOE_ROOT}/tests/setup_bats.bash"
load "${CLOE_ROOT}/tests/setup_testname.bash"

require_program() {
    if ! type $1 &>/dev/null; then
        skip "required program not present: $1"
    fi
}

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

@test "$(testname 'Expect success' 'test_lua05_apply_stack.lua' 'bbee495e-8e19-4ffb-912f-fa75840a7944')" {
    cloe-engine run test_lua05_apply_stack.lua
}

@test "$(testname 'Expect success' 'test_lua06_apply_stack.lua' 'ba5b7fbd-3b47-4767-b7b2-1075bdaa736f')" {
    cloe-engine run test_lua06_apply_stack.lua
}

@test "$(testname 'Expect success' 'test_lua07_schedule_pause.lua' '41ece52e-146a-414d-b93d-4bc4512c49b8')" {
    require_program netcat
    require_program curl

    cloe-engine run test_lua07_schedule_pause.lua
}

@test "$(testname 'Expect success' 'test_lua08_apply_project.lua' '037010ed-7b08-4874-94bd-27d959bdfaca')" {
    cloe-engine run test_lua08_apply_project.lua
}

@test "$(testname 'Expect success' 'test_lua08_apply_project.lua (2)' '037010ed-7b08-4874-94bd-27d959bdfaca')" {
    cd ..
    cloe-engine run tests/test_lua08_apply_project.lua
}

@test "$(testname 'Expect success' 'test_lua09_no_json.lua' '5a0fe683-355c-4584-97ea-fa012f40fa81')" {
    cloe-engine run test_lua09_no_json.lua
}

@test "$(testname 'Expect success' 'test_lua10_heavy_cpu.lua' 'fbf32388-a80e-4fb3-b334-b4cd4f020cdb')" {
    cloe-engine run test_lua10_heavy_cpu.lua
}

@test "$(testname 'Expect success' 'test_lua11_serial_tests.lua' '852edc33-a344-437e-b11d-82527a0ea387')" {
    cloe-engine run test_lua11_serial_tests.lua
}

@test "$(testname 'Expect failure' 'test_lua12_fail_after_stop.lua' '880875e8-b7ad-4d86-abf5-b2cd31b1a1db')" {
    run cloe-engine run test_lua12_fail_after_stop.lua
    assert_check_failure $status $output
}

# --- API ---------------------------------------------------------------------

@test "$(testname 'Check API' 'test_lua_api_cloe_system.lua' '23496512-a7f9-4fb7-8ed3-a655954b24f7')" {
    cloe-engine shell test_lua_api_cloe_system.lua
}

# --- Better errors -----------------------------------------------------------

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
