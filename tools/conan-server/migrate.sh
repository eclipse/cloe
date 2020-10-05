#!/bin/bash
#
# Usage: migrate.sh SRC_REMOTE DST_REMOTE
#

set -e

if [ $# -ne 2 ]; then
    echo "Error: require exactly two arguments: SRC_REMOTE and DST_REMOTE" >&2
    exit 1
fi

src_remote=$1
dst_remote=$2

for package in $(conan search -r $src_remote --raw); do
    # Fix package names that are not fully specified.
    if ! (echo "${package}" | grep -q "@"); then
        package="${package}@_/_"
    fi

    echo "Migrating: ${package} ..."
    conan download -r "$src_remote" "${package}"
    conan upload -r "$dst_remote" --all --confirm --no-overwrite all "${package}"
done
