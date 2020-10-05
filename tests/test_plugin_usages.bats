#!/usr/bin/env bats

CLOE_LOG_LEVEL=info

load setup_bats

setup() {
    OLDIFS="${IFS}"
    IFS=':' plugin_path=(${CLOE_PLUGIN_PATH})
    IFS="${OLDIFS}"
    cloe_plugins=()
    for path in ${plugin_path[@]}; do
        cloe_plugins+=(${path}/*.so)
    done
    cloe_plugins+=("builtin://simulator/nop" "builtin://controller/nop")
    export cloe_plugins
}

@test "Test plugin usage message output" {
    for file in "${cloe_plugins[@]}"; do
        echo "Checking: $file"
        run cloe_engine usage $file
        if [[ $status -ne 0 ]]; then
            echo -e $output
        fi
        assert_exit_success $status
        assert_not_empty $output
    done
}

@test "Test plugin JSON schema output" {
    if ! type jq &>/dev/null; then
        skip "required program jq not present"
    fi
    skip "not ready yet"

    for file in "${cloe_plugins[@]}"; do
        echo "Checking: $file"
        run cloe_engine usage -j $file
        if [[ $status -ne 0 ]]; then
            echo -e $output
        fi
        assert_exit_success $status
        assert_not_empty $output
        echo $output | jq -Me .
    done
}
