#!/usr/bin/env bats

load setup_bats
load setup_testname

# Usage: FILENAME=$(mktemp_gndtruth_out SUFFIX)
#
# Prints a new temporary filename with the given suffix.
# This allows different tests be run in parallel.
mktemp_gndtruth_out() {
    local suffix=${1}
    mktemp -u --suffix=.${suffix} /tmp/cloe-gndtruth-XXXXX
}

# Usage: fn_test_gndtruth SUFFIX DECOMPRESS
#
# Parameters:
#   SUFFIX      One of "json", "json.gz", "json.bz2", "msgpack", "msgpack.gz", "msgpack.bz2"
#   DECOMPRESS  One of "cat", "gunzip", "bunzip2"
fn_test_gndtruth() {
    local suffix="$1"
    local decompress="$2"

    # This neat little trick will give us "json" or "msgpack"
    local base_suffix="${suffix/.*/}"
    local CLOE_GNDTRUTH_REF="test_gndtruth_smoketest_output.${base_suffix}"

    export CLOE_GNDTRUTH_OTYPE="$suffix"
    export CLOE_GNDTRUTH_ONAME="$(mktemp_gndtruth_out $suffix)"

    # Clean any pre-existing file.
    if [ -f "$CLOE_GNDTRUTH_ONAME" ]; then
        rm "$CLOE_GNDTRUTH_ONAME"
    fi

    cloe-engine run test_gndtruth_smoketest.json
    assert_exit_success $?

    # Expect configured output file to exist.
    test -f "$CLOE_GNDTRUTH_ONAME"

    # Expect reference file to equal the "decompressed" output file
    diff "$CLOE_GNDTRUTH_REF" <($decompress - < "$CLOE_GNDTRUTH_ONAME")

    # Remove temporary file.
    rm "$CLOE_GNDTRUTH_ONAME"
}

@test "$(testname 'Expect json' 'test_gndtruth_smoketest.json' 'd5f58690-e7bc-4170-ab7e-9b785a5866c3')" {
    fn_test_gndtruth "json" "cat"
}

@test "$(testname 'Expect json.gz' 'test_gndtruth_smoketest.json' 'b3a11bb5-6fd1-401e-b9fb-db76b4c338e5')" {
    fn_test_gndtruth "json.gz" "gunzip"
}

@test "$(testname 'Expect json.bz2' 'test_gndtruth_smoketest.json' '69eeb436-4b5e-4a31-9097-01f03cb0f71d')" {
    fn_test_gndtruth "json.bz2" "bunzip2"
}

@test "$(testname 'Expect msgpack' 'test_gndtruth_smoketest.json' '40757b44-3aa0-4a46-8597-ebf2b8acfbab')" {
    fn_test_gndtruth "msgpack" "cat"
}

@test "$(testname 'Expect msgpack.gz' 'test_gndtruth_smoketest.json' 'bf77683f-7c1a-4d70-b713-e26f440e71f7')" {
    fn_test_gndtruth "msgpack.gz" "gunzip"
}

@test "$(testname 'Expect msgpack.bz2' 'test_gndtruth_smoketest.json' 'c50350d5-9b63-43bb-9e00-d31646e50b0a')" {
    fn_test_gndtruth "msgpack.bz2" "bunzip2"
}

@test "$(testname 'Expect run failure' 'test_gndtruth_invalid_file.json' '29e8ff3b-86f3-4320-ba69-05600df1f836')" {
    cloe-engine check test_gndtruth_invalid_file.json
    run cloe-engine run test_gndtruth_invalid_file.json
    assert_exit_failure $status
    test $status -eq $CLOE_EXIT_ABORTED
}
