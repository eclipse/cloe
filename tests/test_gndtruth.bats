#!/usr/bin/env bats

load setup_bats

################################################################################
# fn_cleanup                                                                   #
################################################################################
# ---------------------------------------------------------------------------- #
#   Parameter 1: Extension of a groundtruth file which shall be deleted        #
# Postcondition: /tmp/cloe_gndtruth.$1 deleted                                 #
# Postcondition: /tmp/cloe_gndtruth.json deleted                               #
# Postcondition: /tmp/cloe_gndtruth.msg.$1 deleted                             #
################################################################################

fn_cleanup() {
    rm -f /tmp/cloe_gndtruth.json
    rm -f /tmp/cloe_gndtruth.msg
    [[ -z $1 ]] || rm -f /tmp/cloe_gndtruth.$1
}

################################################################################
# fn_run_and_test_compression                                                  #
################################################################################
# ---------------------------------------------------------------------------- #
#   Parameter 1: Extension of the groundtruth file being created               #
#   Parameter 2: Ordinal number of the testcase                                #
#   Parameter 3: Output of 'file' to check for                                 #
#  Precondition: cloe_engine is operable                                       #
#  Precondition: json test test_gndtruth_smoketest-$2 exists                   #
#  Precondition: Linux application 'file' & 'grep' are available               #
# Postcondition: /tmp/cloe_gndtruth.*.$1 was tested                            #
#          Note: Tested is occurrence of the gndtruth_lane_sensor              #
################################################################################

fn_run_and_test_compression() {
    fn_cleanup "$1"
    cloe_engine run test_gndtruth_smoketest-$2.json
    file /tmp/cloe_gndtruth.$1 | grep "$3"
}

################################################################################
# fn_unpack_gz                                                                 #
################################################################################
# Unpacks a gzip file                                                          #
# ---------------------------------------------------------------------------- #
#   Parameter 1: extension of the filename (after /tmp/cloe_gndtruth.)         #
#  Precondition: gzip file /tmp/cloe_gndtruth.$1 exists                        #
# Postcondition: /tmp/cloe_gndtruth.*.$1 is unpacked                           #
################################################################################

fn_unpack_gz() {
    gunzip /tmp/cloe_gndtruth.$1
}

################################################################################
# fn_unpack_bz2                                                                #
################################################################################
# Unpacks a bzip2 file                                                         #
# ---------------------------------------------------------------------------- #
#   Parameter 1: extension of the filename (after /tmp/cloe_gndtruth.)         #
#  Precondition: bzip2 file /tmp/cloe_gndtruth.$1 exists                       #
# Postcondition: /tmp/cloe_gndtruth.*.$1 is unpacked                           #
################################################################################

fn_unpack_bz2() {
    bunzip2 /tmp/cloe_gndtruth.$1
}

################################################################################
# fn_test_gndtruth                                                             #
################################################################################
# Minimal test for properties of a gndtruth file                               #
# ---------------------------------------------------------------------------- #
#   Parameter 1: extension of the filename (after /tmp/cloe_gndtruth.)         #
#  Precondition: bzip2 file /tmp/cloe_gndtruth.$1 exists                       #
# Postcondition: /tmp/cloe_gndtruth.*.$1 was tested                            #
#          Note: Tested is occurrence of the gndtruth_lane_sensor              #
#          Note: Tested is occurrence of the default_world_sensor              #
#          Note: EXPECTED_CYCLE_COUNT := 1s / 20ms/Cycle - 1 Cycle             #
################################################################################

fn_test_gndtruth() {
    local suffix=$1;
    local tmpfile=/tmp/cloe_gndtruth.$suffix;
    local EXPECTED_CYCLE_COUNT=49;
    local ACTUAL_CYCLE_COUNT=$(cat $tmpfile | grep -ao "cloe::gndtruth_lane_sensor" | wc -l)
    if [[ "$ACTUAL_CYCLE_COUNT" != "$EXPECTED_CYCLE_COUNT" ]]; then
        exit 1;
    fi;
    local ACTUAL_CYCLE_COUNT=$(cat /tmp/cloe_gndtruth.$1 | grep -ao "cloe::default_world_sensor" | wc -l)
    if [[ "$ACTUAL_CYCLE_COUNT" != "$EXPECTED_CYCLE_COUNT" ]]; then
        exit 1;
    fi;
    fn_cleanup
}

@test "Expect json.gz : test_gndtruth_smoketest-json-gz.json (Test-ID b3a11bb5-6fd1-401e-b9fb-db76b4c338e5)" {
    fn_run_and_test_compression json.gz "json-gz" "gzip compressed data"
    fn_unpack_gz json.gz
    fn_test_gndtruth json
}
@test "Expect json.bz2: test_gndtruth_smoketest-2.json (Test-ID d5f58690-e7bc-4170-ab7e-9b785a5866c3)" {
    fn_run_and_test_compression json.bz2 "json-bz2" "bzip2 compressed data"
    fn_unpack_bz2 json.bz2
    fn_test_gndtruth json
}
@test "Expect json    : test_gndtruth_smoketest-3.json (Test-ID 69eeb436-4b5e-4a31-9097-01f03cb0f71d)" {
    fn_run_and_test_compression json "json" "ASCII text"
    fn_test_gndtruth json
}
@test "Expect msg.gz  : test_gndtruth_smoketest-4.json (Test-ID 40757b44-3aa0-4a46-8597-ebf2b8acfbab)" {
    fn_run_and_test_compression msg.gz "msg-gz" "gzip compressed data"
    fn_unpack_gz msg.gz
    fn_test_gndtruth msg
}
@test "Expect msg.gz  : test_gndtruth_smoketest-5.json (Test-ID bf77683f-7c1a-4d70-b713-e26f440e71f7)" {
    fn_run_and_test_compression msg.bz2 "msg-bz2" "bzip2 compressed data"
    fn_unpack_bz2 msg.bz2
    fn_test_gndtruth msg
}
@test "Expect msg.gz  : test_gndtruth_smoketest-6.json (Test-ID c50350d5-9b63-43bb-9e00-d31646e50b0a)" {
    fn_run_and_test_compression msg "msg" "data"
    fn_test_gndtruth msg
}
