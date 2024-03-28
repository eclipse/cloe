#!/usr/bin/env bats

load setup_bats
load setup_testname

@test "$(testname 'Expect check success' 'test_virtue_missing_lanes_fail.json' 'd9072cc3-62f3-4f11-bf9e-b514fea67e4b')" {
    cloe-engine check test_virtue_missing_lanes_fail.json
}

# shall fail because nop simulator provides no lanes
@test "$(testname 'Expect run failure' 'test_virtue_missing_lanes_fail.json' '661c7f7b-32b5-4609-9caa-930b956596d3')" {
    ! cloe-engine run test_virtue_missing_lanes_fail.json
}

@test "$(testname 'Expect check success' 'test_virtue_missing_lanes_pass.json' '96aeec09-b5de-4a1e-8a29-a481315c0735')" {
    cloe-engine check test_virtue_missing_lanes_pass.json
}

# shall pass because minimator provides lanes
@test "$(testname 'Expect run success' 'test_virtue_missing_lanes_pass.json' '745efe30-621e-4231-8a8f-7da6a833a881')" {
    cloe-engine run test_virtue_missing_lanes_pass.json
}
