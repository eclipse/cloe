#!/bin/bash
#
# This script generates the entries for a changelog.
#
# Usage Examples:
#   changelog-gen.sh v0.18.0..HEAD
#

git log --pretty='format:%s `[%h] <https://github.com/eclipse/cloe/commit/%H>`_' --decorate "$@" \
    | sed -r '/Merge pull request/d' \
    | sort -t: -k1,1 -s
