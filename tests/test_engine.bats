#!/usr/bin/env bats

load setup_bats
load setup_testname

@test "$(testname 'Expect schema equality' 'test_engine_json_schema.json' '4d368665-b666-4289-8a7a-b76ca53db688')" {
    # Note: you will have to update the schema files every time you change the schema,
    # in order that this test completes successfully. Here's how you do it.
    #
    #    cloe-launch -vv exec conanfile.py -- usage -j > tests/test_engine_json_schema.json
    #
    # Then use git diff to check that the changes make sense and are what you expect.
    require_program diff

    diff <(cloe-engine usage -j 2>/dev/null) test_engine_json_schema.json
}

@test "$(testname 'Expect stack equality' 'test_engine_nop_smoketest_dump.json' '3b23bb69-b249-49c8-8b4c-2fa993d8677e')" {
    require_program diff

    # The plugin section and the registry section have path references, which
    # are not reproducible, so we need to rewrite them and delete these lines.
    # This does not negatively affect the validity of the test.
    reference_file=test_engine_nop_smoketest_dump.json
    diff <(cloe-engine dump config_nop_smoketest.json |
           sed -r -e '#"/.*\/.*.conan/data/([^/]+)/.*"#d' \
                  -e '\#\.\.\./cloe-engine/\.\.\.#d' \
                  -e "\\#(${HOME-/root}|${CONAN_USER_HOME-/cloe_dev})/.*#d" \
          ) \
          ${reference_file}
}

@test "$(testname 'Expect check success' 'test_engine_smoketest.json' '20c3f11e-4a93-4066-b61e-d485be5c8979')" {
    cloe-engine check test_engine_smoketest.json
}

@test "$(testname 'Expect run success' 'test_engine_smoketest.json' 'b590e751-dace-4139-913c-a4a812af70ac')" {
    cloe-engine run test_engine_smoketest.json
}

@test "$(testname 'Expect check failure' 'test_engine_bad_logging.json' '107c36fe-7bd9-4559-b5e9-74b72baafd9f')" {
    run cloe-engine check test_bad_logging.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect run failure' 'test_engine_invalid_trigger.json' '0cf8c9e0-5538-4969-a00e-4891d7d8e647')" {
    # Currently, cloe-engine cannot tell before starting a simulation
    # whether the triggers exist or not.
    cloe-engine check test_engine_invalid_trigger.json

    run cloe-engine run test_engine_invalid_trigger.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_ABORTED
}

@test "$(testname 'Expect run success' 'test_engine_optional_trigger.json' 'c630a68c-8b1e-436a-8acf-b282b1e1830c')" {
    cloe-engine check test_engine_optional_trigger.json
    cloe-engine run test_engine_optional_trigger.json
}

@test "$(testname 'Expect check success' 'test_engine_curl_succeed.json' '5eff0c85-77f1-4792-9987-e46a36617d99')" {
    cloe-engine check test_engine_curl_succeed.json
}

@test "$(testname 'Expect run success' 'test_engine_curl_succeed.json' 'f473cb96-7f2e-4ac1-801a-fd93343f6e24')" {
    require_program curl
    require_engine_with_server

    cloe-engine run test_engine_curl_succeed.json
}

@test "$(testname 'Expect run failure' 'test_engine_curl_succeed.json' '7aa4b455-ad74-4e5f-bf82-43761d7cd81b')" {
    # When disabling the `enable_command_action` flag, curl
    # should not be used and the simulation should fail by default.
    run cloe-engine run test_engine_curl_succeed.json \
        <(echo '{ "version": "4", "engine": { "security": { "enable_command_action": false } } }')
    test $status -eq $CLOE_EXIT_FAILURE
}

@test "$(testname 'Expect check failure' 'test_engine_empty.json' 'd04369fb-e4af-4f80-aaa8-8352d3dec42e')" {
    run cloe-engine check test_engine_empty.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "$(testname 'Expect check failure' 'test_engine_version_absent.json' '4de20aed-2a8f-442b-bdb3-1da76237ee1e')" {
    run cloe-engine check test_engine_version_absent.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "$(testname 'Expect check failure' 'test_engine_version_wrong.json' '2c883a3d-b877-4fca-88a1-8078208f0d2b')" {
    run cloe-engine check test_engine_version_wrong.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
    test $(echo $output 2>&1 | wc -l) -lt 32
}

@test "$(testname 'Expect check success' 'test_engine_fail_trigger.json' 'cd540e21-63e9-421f-8e7b-e113339253a2')" {
    cloe-engine check test_engine_fail_trigger.json
}

@test "$(testname 'Expect run failure' 'test_engine_fail_trigger.json' 'c9bc16de-4902-4abf-90ac-d4bef0d0b26e')" {
    run cloe-engine run test_engine_fail_trigger.json
    assert_exit_failure $status
    test $status -eq $CLOE_EXIT_FAILURE
    echo "$output" | grep '"outcome": "failure"'
}

@test "$(testname 'Expect check failure' 'test_engine_hook_invalid.json' 'cc3d8879-5ca5-4ffb-a78e-c9723ac82b91')" {
    run cloe-engine check test_engine_hook_invalid.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check failure' 'test_engine_hook_noexec.json' '3f35bb1f-307a-42e3-a2ea-739e60cab084')" {
    run cloe-engine check test_engine_hook_noexec.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check success' 'test_engine_hook_failure.json' 'd6d8fe35-c513-4e43-8984-e9da0958c0fd')" {
    cloe-engine check test_engine_hook_failure.json
}

@test "$(testname 'Expect run failure' 'test_engine_hook_failure.json' '798ee383-667d-4971-af2f-51c49c47b750')" {
    run cloe-engine run test_engine_hook_failure.json
    test $status -eq $CLOE_EXIT_ABORTED
}

@test "$(testname 'Expect check success' 'test_engine_hook_ok.json' '2e3064bf-0e6d-4934-b25f-78cae54bd4d1')" {
    cloe-engine check test_engine_hook_ok.json
}

@test "$(testname 'Expect run success' 'test_engine_hook_ok.json' '303afa7e-3ebe-4db3-a802-84b2cdba97c5')" {
    cloe-engine run test_engine_hook_ok.json
}

@test "$(testname 'Expect check success' 'test_engine_ignore.json' '78a470a4-cbe1-4436-a43f-d956685b8bc9')" {
    cloe-engine check test_engine_ignore.json
}

@test "$(testname 'Expect run success' 'test_engine_ignore.json' '1738bf42-3784-4e9c-b4d4-76e94c8e5271')" {
    cloe-engine run test_engine_ignore.json
}

@test "$(testname 'Expect check failure' 'test_engine_include_nonexistent.json' 'bad115cc-0397-48e6-9a51-bdcfeaf6b024')" {
    run cloe-engine check test_engine_include_nonexistent.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check failure' 'test_engine_include_self.json' '7b3e4010-19a0-4518-b8d7-00655af93755')" {
    run cloe-engine check test_engine_include_self.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check failure' 'test_engine_incomplete.json' '8baa711a-9279-4527-9ecc-6b685551a45f')" {
    run cloe-engine check test_engine_incomplete.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect run failure' 'test_engine_incomplete.json' '75a0c642-2373-4d6d-bd40-61e0f7840c57')" {
    run cloe-engine run test_engine_incomplete.json
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check success' 'test_engine_keep_alive.json' '254544dc-c17a-4a5c-8685-723ed1c758cf')" {
    require_program kill

    cloe-engine check test_engine_keep_alive.json
}

@test "$(testname 'Expect run success' 'test_engine_keep_alive.json' '0c5ace05-f5ca-4615-9c14-62a75b69651a')" {
    require_program kill

    cloe-engine run test_engine_keep_alive.json
}

@test "$(testname 'Expect check success' 'test_engine_namespaced_smoketest.json' 'c9d4f8d3-aec7-404e-95f5-7a14cd8674c7')" {
    cloe-engine check test_engine_namespaced_smoketest.json
}

@test "$(testname 'Expect run success' 'test_engine_namespaced_smoketest.json' 'c2fd481b-f135-4fb5-86f0-699af0f93497')" {
    cloe-engine run test_engine_namespaced_smoketest.json
}

@test "$(testname 'Expect check failure' 'test_engine_no_binding.json' '111a4f1f-7679-48a8-88de-a6773dab055e')" {
    run cloe-engine check test_engine_no_binding.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect run failure' 'test_engine_no_binding.json' 'dc682e38-38e5-43d7-8989-b477cb0f6c2a')" {
    run cloe-engine run test_engine_no_binding.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check failure' 'test_engine_no_vehicle.json' '319a342a-88d2-4b07-a4de-1fef78b13d72')" {
    run cloe-engine check test_engine_no_vehicle.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect run failure' 'test_engine_no_vehicle.json' '4b4fcd8b-add1-451c-ad7b-9c0a3efb6525')" {
    run cloe-engine run test_engine_no_vehicle.json
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check failure' 'test_engine_unknown_vehicle.json' '3f754bea-4806-4f43-9c60-a78b13b43f6f')" {
    run cloe-engine check test_engine_unknown_vehicle.json
    assert_check_failure $status $output
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect run failure' 'test_engine_unknown_vehicle.json' 'da3d7c55-6024-481b-bee4-32233df6c330')" {
    run cloe-engine run test_engine_unknown_vehicle.json
    test $status -eq $CLOE_EXIT_UNKNOWN
}

@test "$(testname 'Expect check success' 'test_engine_pause.json' '24528c78-f681-44ff-8b9c-5c172acf19b9')" {
    cloe-engine check test_engine_pause.json
}

@test "$(testname 'Expect run success' 'test_engine_pause.json' '845e3c9b-2a6d-469a-93a7-67fe9531c81e')" {
    require_program curl
    require_engine_with_server

    cloe-engine run test_engine_pause.json
}

@test "$(testname 'Expect check success' 'test_engine_sticky_trigger.json' '76400941-50f5-4185-9107-335005079569')" {
    cloe-engine check test_engine_sticky_trigger.json
}

@test "$(testname 'Expect run success' 'test_engine_sticky_trigger.json' 'dcfbdd81-8bb0-4de3-ac68-486aa5a9ce66')" {
    skip "not implemented yet"
    cloe-engine run test_engine_sticky_trigger.json
}

@test "$(testname 'Expect check success' 'test_engine_stuck_controller.json' '72f151f7-26c3-4a79-a0ec-8b0d4e678442')" {
    cloe-engine check test_engine_stuck_controller.json
}

@test "$(testname 'Expect run aborted' 'test_engine_stuck_controller.json' '13a244da-f18e-4dec-af5c-d5dcb4c2cd36')" {
    run cloe-engine run test_engine_stuck_controller.json
    assert_exit_failure $status
    echo "$output" | grep '"outcome": "aborted"'
}

@test "$(testname 'Expect check success' 'test_engine_stuck_controller_continue.json' '19b3d407-c980-4507-9bd2-9d6dca5e1320')" {
    cloe-engine check test_engine_stuck_controller_continue.json
}

@test "$(testname 'Expect run success' 'test_engine_stuck_controller_continue.json' '5692f85d-87a2-4866-9875-647802ce1d62')" {
    cloe-engine run test_engine_stuck_controller_continue.json
}

@test "$(testname 'Expect check success' 'test_engine_watchdog.json' '30e097cf-6e51-484b-a212-690769cd4c91')" {
    cloe-engine check test_engine_watchdog.json
}

@test "$(testname 'Expect run syskill' 'test_engine_watchdog.json' '058ff9b7-98dc-4583-8e80-c70e9c5e1f4e')" {
    require_program curl
    require_engine_with_server

    cloe-engine check test_engine_watchdog.json

    # assert abort with core dump, code 134 (bash) / 250 (python)
    run cloe-engine run test_engine_watchdog.json
    test $status -eq $CLOE_EXIT_SYSKILL
}

@test "$(testname 'Expect check/run success' 'test_engine_smoketest.json [ts=5ms]' '1a31022c-e20c-4a9e-9373-ad54a3729442')" {
    local timestep_stack="option_timestep_5.json"
    cloe-engine check test_engine_smoketest.json "${timestep_stack}"
    cloe-engine run test_engine_smoketest.json "${timestep_stack}"
}

@test "$(testname 'Expect check/run success' 'test_engine_smoketest.json [ts=60ms]' 'e7957fa0-1145-4458-b665-eec51c1f0da5')" {
    local timestep_stack="option_timestep_60.json"
    cloe-engine check test_engine_smoketest.json "${timestep_stack}"
    cloe-engine run test_engine_smoketest.json "${timestep_stack}"
}
