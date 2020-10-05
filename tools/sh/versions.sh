#!/bin/bash

version_le() {
    [[ "$1" == "$(printf "$1\n$2" | sort -V | head -n1)" ]]
}

version_lt() {
    [[ "$1" == "$2" ]] && false || version_le $1 $2
}

cmake_version() {
    cmake --version | head -1 | cut -d' ' -f3
}

# Returns the codename of the current Linux release, such as bionic or xenial.
lsb_codename() {
    (
        source /etc/lsb-release
        cat ${DISTRIB_CODENAME}
    )
}

system_cores() {
    local default_cores=${1-1}
    if hash lscpu; then
        lscpu | sed  -nr "s/^CPU\(s\):\s+//p"
    else
        echo $default_cores
    fi
}

# Returns one of: linux, mingw32, mingw64
#
# It is of course very well possible for it to return something else,
# which is then not supported.
system_codename() {
    uname | sed 's/_NT.*//' | tr [:upper:] [:lower:]
}

# Returns true if system is in a MinGW environment.
#
# Use like so:
#
#    if system_is_mingw; then
#       ...
#    fi
system_is_mingw() {
    test $(system_codename) != "linux"
}

# Returns true if system should target 64-bit environment.
#
# Use like so:
#
#    if system_is_amd64; then
#       ...
#    fi
system_is_amd64() {
    test $(system_codename) != "mingw32"
}
