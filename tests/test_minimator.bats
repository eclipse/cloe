#!/usr/bin/env bats

load setup_bats
load setup_testname

@test "$(testname 'Expect check success' 'test_minimator_smoketest.json' 'c7a427e7-eb2b-4ae7-85ec-a35b7540d4aa')" {
    cloe-engine check test_minimator_smoketest.json
}

@test "$(testname 'Expect run success' 'test_minimator_smoketest.json' '7c67ceb9-3d1d-47e4-9342-0b39099c59d6')" {
    cloe-engine run test_minimator_smoketest.json
}

@test "$(testname 'Expect check/run success' 'test_minimator_smoketest.json [ts=5ms]' '57254185-5480-4859-b2a5-6c3a211a22e0')" {
    local timestep_stack="option_timestep_5.json"
    cloe-engine check test_minimator_smoketest.json "${timestep_stack}"
    cloe-engine run test_minimator_smoketest.json "${timestep_stack}"
}

@test "$(testname 'Expect check/run success' 'test_minimator_smoketest.json [ts=60ms]' 'a0d4982f-8c02-4759-bc88-cc30a1ccbbf0')" {
    local timestep_stack="option_timestep_60.json"
    cloe-engine check test_minimator_smoketest.json "${timestep_stack}"
    cloe-engine run test_minimator_smoketest.json "${timestep_stack}"
}

@test "$(testname 'Expect check/run success' 'test_minimator_multi_agent_smoketest.json' '90e440ec-e8bc-40bb-8d2a-de224ee872bb')" {
    cloe-engine check test_minimator_multi_agent_smoketest.json
    cloe-engine run test_minimator_multi_agent_smoketest.json
}
