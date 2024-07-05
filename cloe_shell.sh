#!/bin/bash

export CLOE_LOG_LEVEL=debug
export CLOE_STRICT_MODE=1
export CLOE_ROOT="$(pwd)"
export CLOE_ENGINE="${CLOE_ROOT}/$1"
export CLOE_LUA_PATH="${CLOE_ROOT}/engine/lua"
export PATH="$(dirname "$CLOE_ENGINE"):$PATH"

# Set plugin paths
shift 1
while [[ $# -ne 0 ]]; do
    CLOE_PLUGIN_PATH="${CLOE_ROOT}/$(dirname "$1"):$CLOE_PLUGIN_PATH"
    shift 1
done
export CLOE_PLUGIN_PATH

exec $SHELL
