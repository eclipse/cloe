#!/usr/bin/env bats

cd ${BATS_TEST_DIRNAME}
export CLOE_ROOT="${BATS_TEST_DIRNAME}/.."

export exit_outcome_success=0
export exit_outcome_unknown=1
export exit_outcome_nostart=4
export exit_outcome_stopped=8
export exit_outcome_failure=9
export exit_outcome_aborted=16

# This exit outcome is different for each environment.
# In Python it's 250 and in Bash it's 134, for example.
export exit_outcome_syskill=250

# We now only support using cloe-launch to run tests. This has several
# reasons:
#
#   1. We can use plugin module.py configuration for smoketests.
#   2. It forces us to support all use-cases with cloe-launch.
#   3. It provides one clear way to do things.
#
# The following extra variables are read:
#
#   CLOE_LOG_LEVEL      : string
#   CLOE_PROFILE_FILE   : path
#   CLOE_OVERRIDE_ENV   : array of strings
#   CLOE_WRITE_OUTPUT   : bool
cloe_engine() {
    local cloe_log_level="${CLOE_LOG_LEVEL-debug}"
    local cloe_profile_file="${CLOE_PROFILE_FILE-${CLOE_ROOT}/conanfile.py}"
    local cloe_launch_exe=(python3 -m cloe_launch)
    local cloe_launch_args=(-vv exec -c -P "${cloe_profile_file}")
    local cloe_engine_args=(--no-system-confs --no-system-plugins --level ${cloe_log_level})

    for var in ${CLOE_OVERRIDE_ENV[@]}; do
        cloe_launch_args+=(--override-env=${var})
    done

    local user_args=()
    while [[ $# -ne 0 ]]; do
        case $1 in
            run)
                if [[ -n ${CLOE_WRITE_OUTPUT} ]]; then
                    user_args+=(run --require-success "${BATS_OPTIONAL_STACKS}")
                else
                    user_args+=(run --no-write-output --require-success "${BATS_OPTIONAL_STACKS}")
                fi
                ;;
            check)
                user_args+=(check "${BATS_OPTIONAL_STACKS}")
                ;;
            *)
                user_args+=("$1")
                ;;
        esac
        shift
    done

    # Run the prepared command:
    (
        echo "Running:" "${cloe_launch_exe[@]}" "${cloe_launch_args[@]}" -- "${cloe_engine_args[@]}" "${user_args[@]}" >&2
        export PYTHONPATH="${CLOE_ROOT}/cli"
        "${cloe_launch_exe[@]}" "${cloe_launch_args[@]}" -- "${cloe_engine_args[@]}" "${user_args[@]}"
    )
}

cloe_shell() {
    local cloe_log_level="${CLOE_LOG_LEVEL-debug}"
    local cloe_profile_file="${CLOE_PROFILE_FILE-${CLOE_ROOT}/conanfile.py}"
    local cloe_launch_exe=(python3 -m cloe_launch)
    local cloe_launch_args=(-vv shell -c -P "${cloe_profile_file}")

    for var in ${CLOE_OVERRIDE_ENV[@]}; do
        cloe_launch_args+=(--override-env=${var})
    done

    local user_args=("$@")

    # Run the prepared command:
    (
        echo "Running:" "${cloe_launch_exe[@]}" "${cloe_launch_args[@]}" -- "${user_args[@]}" >&2
        export PYTHONPATH="${CLOE_ROOT}/cli"
        "${cloe_launch_exe[@]}" "${cloe_launch_args[@]}" -- "${user_args[@]}"
    )
}

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

    cloe_engine usage "${plugin}" &>/dev/null
}
