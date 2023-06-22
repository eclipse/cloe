#!/usr/bin/env bats

CLOE_LOG_LEVEL=info

load setup_bats
load setup_testname

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

@test "$(testname 'Expect run success' 'cloe-engine usage' '93c5a785-8f07-4df2-92ee-6d47e728f201')" {
    for file in "${cloe_plugins[@]}"; do
        echo "Checking: $file"
        run cloe-engine usage $file
        if [[ $status -ne 0 ]]; then
            echo -e $output
        fi
        assert_exit_success $status
        assert_not_empty $output
    done
}

@test "$(testname 'Expect run success' 'cloe-engine usage -j' '7576ad69-1d16-43f8-9809-af0bf408e4f1')" {
    if ! type jq &>/dev/null; then
        skip "required program jq not present"
    fi
    skip "not ready yet"

    for file in "${cloe_plugins[@]}"; do
        echo "Checking: $file"
        run cloe-engine usage -j $file
        if [[ $status -ne 0 ]]; then
            echo -e $output
        fi
        assert_exit_success $status
        assert_not_empty $output
        echo $output | jq -Me .
    done
}
