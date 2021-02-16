#!/usr/bin/env bats

load setup_bats

teardown_vtd() {
    # CLOE_ROOT is only available outside the shell.
    cloe_shell -c "${CLOE_ROOT}/plugins/vtd/bin/vtd stop"
}

test_vtd_plugin_exists() {
    # VTD_ROOT is only available in the shell.
    cloe_shell -c 'test -d "${VTD_ROOT}"'
}

teardown() {
    # It's harmless to stop vtd multiple times, so do it in case something goes wrong.
    if test_vtd_plugin_exists; then
        echo "Teardown VTD (from BATS)"
        teardown_vtd
    fi
}

@test "Expect check/run success: test_vtd_smoketest.json" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest.json
    cloe_engine run test_vtd_smoketest.json
}

@test "Expect check/run success: test_vtd_watchdog.json" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    if ! type killall &>/dev/null; then
        skip "required program killall not present"
    fi

    cloe_engine check test_vtd_watchdog.json

    # assert abort with core dump, code 134/250
    run cloe_engine run test_vtd_watchdog.json
    test $status -eq $exit_outcome_syskill
}

@test "Expect check/run success: test_vtd_smoketest.json [ts=5ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_5.json"
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest.json
    cloe_engine run test_vtd_smoketest.json
}

@test "Expect check/run success: test_vtd_smoketest.json [ts=60ms]" {
    BATS_OPTIONAL_STACKS="option_timestep_60.json"
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest.json
    cloe_engine run test_vtd_smoketest.json
}

@test "Expect run success      : test_vtd_clean_timeout.json" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi

    run cloe_engine run test_vtd_clean_timeout.json
    assert_check_failure $status $output
    # This will look weird, that because of BATS.
    echo $output
    test $status -eq $exit_outcome_unknown
    echo $output | grep '### test successful ###'
}

# Note our open support request regarding stimulating multiple external
# vehicles in VTD: https://redmine.vires.com/issues/13340
#
# TODO: Improve the tested condition once we now how to correctly deal with VTD
@test "Expect check/run success: test_vtd_multi_agent_smoketest.json" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    cloe_engine check test_vtd_multi_agent_smoketest.json
    cloe_engine run test_vtd_multi_agent_smoketest.json
}

@test "Expect run failure      : test_vtd_unknown_sensor.json" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi

    run cloe_engine run test_vtd_unknown_sensor.json
    test $status -eq $exit_outcome_unknown
}

@test "Expect check/run success: test_gndtruth_smoketest.json" {
    if ! test_plugin_exists gndtruth_extractor; then
        skip "required controller gndtruth_extractor not present"
    elif ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi

    local destfile="/tmp/cloe_gndtruth.json.gz"
    if [[ -f $destfile ]]; then
        rm $destfile
    fi
    cloe_engine check test_gndtruth_smoketest.json
    cloe_engine run test_gndtruth_smoketest.json
    test -s $destfile
    rm $destfile
}

# --------------------------------------------------------------------------- #
# The following tests require:
#
#   A VTD installation including the Module Manager Open Simulation Interface
#   (OSI) plugin.
#
# The plugin is available in the VTD wiki and is pre-installed in the Cloe VTD
# container.

test_vtd_osi_plugin_exists() {
    # VTD_ROOT is only available in the shell.
    cloe_shell -c 'test -f ${VTD_ROOT}/Data/Distros/Distro/Plugins/ModuleManager/libModuleOsi3Fmu.so'
}

test_vtd_osi_model_exists() {
    # VTD_ROOT is only available in the shell.
    cloe_shell -c 'test -n "$(echo "${VTD_EXTERNAL_MODELS}" | grep -o "OSMPDummySensor.so")"'
}

@test "Expect check/run success: test_vtd_smoketest_osi.json" {
    if ! test_vtd_plugin_exists; then
        skip "required simulator vtd not present"
    fi
    if ! test_vtd_osi_plugin_exists; then
        skip "required osi plugin for simulator vtd not present"
    fi
    if ! test_vtd_osi_model_exists; then
        skip "required osi sensor model for simulator vtd not present"
    fi
    cloe_engine check test_vtd_smoketest_osi.json
    cloe_engine run test_vtd_smoketest_osi.json
}
