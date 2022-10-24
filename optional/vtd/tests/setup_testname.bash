#!/bin/bash
#
# Each @test in the BATS tests should use this "function" to format
# the name of the test. This ensures uniformity and readability.
#
# This needs to be in its own file in order that it can be used for
# older versions of BATS, which don't export the functions here for
# use during definintion of test names.
#
# The trick then is to source this file before running bats, and with
# newer versions of BATS, we can just load it.

testname() {
    local expect="$1"
    local schema="$2"
    local uuid="$3"
    printf "%-24s: %-46s : %s" "$expect" "$schema" "$uuid"
}

export -f testname
