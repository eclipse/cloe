#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 DST_REMOTE package [package ...]" >&2
    exit 1
fi

# Read the first argument as the destination remote
DST_REMOTE=$1
shift

# Download all packages and put them in the specified remote.
for package in "$@"; do
    conan download "${package}"
    conan upload -r ${DST_REMOTE} "${package}"
done
