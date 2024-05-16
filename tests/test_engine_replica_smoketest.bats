#!/usr/bin/env bats
#
# This test provides a so-so confirmation that running the output
# configuration of a simulation results in roughly the same output
# again, i.e., the simulation is reproducible.

load setup_bats
load setup_testname

teardown() {
    # Remove the temporary registry.
    rm -r "${CLOE_TMP_REGISTRY}" || true
}

@test "$(testname 'Expect exact replication' 'test_engine_replica_smoketest.json' 'f4fa1794-cc78-4ae6-aa5e-6a3ed3c7913c')" {
    # Clean up in case temporary registry already exists.
    if [[ -d "$CLOE_TMP_REGISTRY" ]]; then
        rm -r "$CLOE_TMP_REGISTRY" || true
    fi

    # Run our initial smoketest, writing to the temporary registry.
    cloe-engine run --write-output -u smoketest test_engine_replica_smoketest.json \
        <(echo '{"version":"4","engine":{"registry_path":"${CLOE_TMP_REGISTRY}"}}')

    # Rename the result to original, so we can use the config to create
    # the same output again.
    mv "$CLOE_TMP_REGISTRY/smoketest" "$CLOE_TMP_REGISTRY/original"

    # Run the replica smoketest from the directory itself.
    (
        cd "$CLOE_TMP_REGISTRY/original"
        cloe-engine run --write-output -u smoketest config.json \
            <(echo '{"version":"4","engine":{"registry_path":"${CLOE_TMP_REGISTRY}"}}')
    )
    test -d $CLOE_TMP_REGISTRY/smoketest
    mv "$CLOE_TMP_REGISTRY/smoketest" "$CLOE_TMP_REGISTRY/replica-1"

    # Run the replica smoketest, writing to the same location as the initial
    # one.
    #
    # We have to include the config file from stdin so that the paths
    # are interpreted as from our current working directory; not relative
    # to the stack file.
    (
        tmpdir=$(mktemp -d)
        cd "$tmpdir"
        cat "$CLOE_TMP_REGISTRY/original/config.json" \
            | cloe-engine run --write-output -u smoketest - \
            <(echo '{"version":"4","engine":{"registry_path":"${CLOE_TMP_REGISTRY}"}}')
        cd /
        rmdir "$tmpdir"
    )
    test -d $CLOE_TMP_REGISTRY/smoketest
    mv "$CLOE_TMP_REGISTRY/smoketest" "$CLOE_TMP_REGISTRY/replica-2"

    # Check that the output configuration and trigger files are identical.
    md5sum -c <(md5sum $CLOE_TMP_REGISTRY/original/{config,triggers}.json | sed 's/original/replica-1/')
    md5sum -c <(md5sum $CLOE_TMP_REGISTRY/original/{config,triggers}.json | sed 's/original/replica-2/')

    # Remove the test registry.
    rm -r "$CLOE_TMP_REGISTRY" || true
}
