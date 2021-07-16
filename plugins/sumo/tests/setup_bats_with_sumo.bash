#!/usr/bin/env bats

load setup_bats

test_sumo_plugin_exists() {
    test_plugin_exists sumo
}

test_dir=$(pwd)

setup() {
    sumo -c $test_dir/../contrib/cfg_files/quickstart.sumocfg --remote-port 9898 &
}

teardown() {

    if test_sumo_plugin_exists; then
        echo "Teardown Sumo (from BATS)"
        # TODO: implement teardown like `killall sumo` or in any better way.
    fi
}
