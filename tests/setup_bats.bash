#!/usr/bin/env bats

# The following variables are read automatically by cloe-engine to set defaults
# for flags. They can be overridden by specifying alternative flags.
export CLOE_LOG_LEVEL=${CLOE_LOG_LEVEL-debug}
export CLOE_STRICT_MODE=1
export CLOE_WRITE_OUTPUT=0

# The path to use whenever we need a temporary registry. Whenever we use this, we
# probably want to specify the `--write-output` flag too, otherwise nothing will
# be written to the registry.
export CLOE_TMP_REGISTRY="${HOME}/.cache/cloe/tmp-registry"

# The following exit codes are used by cloe-engine and can be used to assert
# that the outcome of the test is as expected.
export CLOE_EXIT_SUCCESS=0
export CLOE_EXIT_UNKNOWN=1
export CLOE_EXIT_NOSTART=4
export CLOE_EXIT_STOPPED=8
export CLOE_EXIT_FAILURE=9
export CLOE_EXIT_ABORTED=16

# This exit outcome is different for each environment: In Python it's 250 and
# in Bash it's 134, for example. Since we are running cloe-engine directly from
# BATS, we expect 134 when cloe-engine is force-killed.
export CLOE_EXIT_SYSKILL=134

assert_not_empty() {
    [[ $(echo "${@}" | tr -d '[:space:]') != "" ]]
}

assert_exit_failure() {
    [[ $1 -ne 0 ]]
}

assert_exit_success() {
    [[ $1 -eq 0 ]]
}

assert_check_failure() {
    local status="$1"
    shift
    local output="$@"

    assert_exit_failure ${status}
    assert_not_empty ${output}
}

test_plugin_exists() {
    local plugin="$1"

    cloe-engine usage "${plugin}" &>/dev/null
}
