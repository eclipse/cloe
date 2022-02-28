#!/usr/bin/env bats
#
# This test provides a so-so confirmation that running the output
# configuration of a simulation results in roughly the same output
# again, i.e., the simulation is reproducible.

load setup_bats

@test $(testname "Expect exact replication" "test_engine_replica_smoketest.json" "f4fa1794-cc78-4ae6-aa5e-6a3ed3c7913c") {
    # Clean up in case temporary registry already exists.
    if [[ -d "$cloe_tmp_registry" ]]; then
        rm -r "$cloe_tmp_registry" || true
    fi

    # Run our initial smoketest, writing to the temporary registry.
    cloe_engine_with_tmp_registry run -u smoketest test_engine_replica_smoketest.json

    # Rename the result to original, so we can use the config to create
    # the same output again.
    mv "$cloe_tmp_registry/smoketest" "$cloe_tmp_registry/original"

    # Run the replica smoketest from the directory itself.
    (
        cd "$cloe_tmp_registry/original"
        cloe_engine_with_tmp_registry run -u smoketest config.json
    )
    test -d $cloe_tmp_registry/smoketest
    mv "$cloe_tmp_registry/smoketest" "$cloe_tmp_registry/replica-1"

    # Run the replica smoketest, writing to the same location as the initial
    # one.
    #
    # We have to include the config file from stdin so that the paths
    # are interpreted as from our current working directory; not relative
    # to the stack file.
    (
        tmpdir=$(mktemp -d)
        cd "$tmpdir"
        cat "$cloe_tmp_registry/original/config.json" \
            | cloe_engine_with_tmp_registry run -u smoketest -
        cd /
        rmdir "$tmpdir"
    )
    test -d $cloe_tmp_registry/smoketest
    mv "$cloe_tmp_registry/smoketest" "$cloe_tmp_registry/replica-2"

    # Check that the output configuration and trigger files are identical.
    md5sum -c <(md5sum $cloe_tmp_registry/original/{config,triggers}.json | sed 's/original/replica-1/')
    md5sum -c <(md5sum $cloe_tmp_registry/original/{config,triggers}.json | sed 's/original/replica-2/')

    # Remove the test registry.
    rm -r "$cloe_tmp_registry" || true
}
