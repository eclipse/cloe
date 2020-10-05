#!/bin/bash
#
# This file shows how you could automate the installation of VTD
# within your organization, if you don't already have a method.
#
# Search for all instances of FIXME and replace with appropriate
# values.

source $(readlink -f "$(dirname ${BASH_SOURCE[0]})/setup.sh")

vtd_pkgver="2.2.0"
vtd_pkgname="VTD.2.2"
vtd_pkgdate="20170913"
vtd_pkgfile="vtd.$vtd_pkgver.FIXME(name).$vtd_pkgdate.tgz"
vtd_pkghash="755ec1052ff747657f8afd65662160ae"

vtd_pkgurl=""
vtd_user=""
vtd_pass=""
function set_vtd_pkgurl() {
    if [[ $vtd_user == "" ]]; then
        read -p "User: " vtd_user
    fi
    if [[ $vtd_pass == "" ]]; then
        read -sp "Password: " vtd_pass
    fi
    vtd_pkgurl="https://$vtd_user:$vtd_pass@secure.vires.com/workgroups/vtd/FIXME(path)/$vtd_pkgfile"
}

function vtd_check_url() {
    echo ":: CHECKING VTD PACKAGE URL"
    set_vtd_pkgurl

    echo "-> Package:     $vtd_pkgfile"
    echo "-> Downloading: $vtd_pkgurl.md5"
    local output="$(mktemp -d)/md5sum"
    wget -q "$vtd_pkgurl.md5" -O "$output" || exit 1
    echo "-> MD5sum:      $(cat $output | cut -d' ' -f1)"
    rm "$output" && rmdir $(dirname "$output") || exit 1
}

function vtd_download() {
    local downdir=$1
    local output=${2:-$vtd_pkgfile}

    echo ":: DOWNLOADING VTD"

    cd "$downdir"
    if [[ -f "$vtd_pkgfile" ]]; then
        if [[ $(md5sum "$vtd_pkgfile") == "$vtd_pkghash" ]]; then
            echo "-> Found valid VTD installation package, not downloading."
            return
        fi
    fi

    echo "-> A username and a password is needed to initiate the download."
    echo "   If you require access, please contact:"
    echo
    echo "       FIXME(name) <FIXME(email)>"
    echo
    set_vtd_pkgurl

    wget -c "$vtd_pkgurl" -O "$output" || exit 1
    if [[ $(md5sum "$output" | cut -d' ' -f1) != $vtd_pkghash ]]; then
        echo "Warning: md5sum of downloaded file does not match expected sum." >&2
    fi
}

function vtd_install() {
    local pkgfile=$1
    local destdir=$2

    echo ":: INSTALLING VTD"
    if [[ ! -f "$pkgfile" ]]; then
        echo "Error: cannot find source package $pkgfile" >&2
        exit 1
    fi

    if [[ $(echo "$destdir" | grep -e "^/opt") != "" ]]; then
        echo "Warning: it is not recommended to install VTD system wide."
        read -n 1 -p "Are you sure you want to continue? [y/N] "
        if [[ $REPLY != "y" && $REPLY != "Y" ]]; then
            exit 1
        fi
    fi

    local vtddir=$destdir/$vtd_pkgname
    if [[ -d "$vtddir" ]]; then
        echo "Error: destination directory already exists: $vtddir" >&2
        exit 1
    fi

    tar xf "$pkgfile" -C "$destdir" || exit 1
}

function vtd_configure() {
    cat <<EOF
:: CONFIGURING VTD
-> Before you can use VTD, you need to either
     a) install a license, or
     b) use a license server.
   You can set a license server by setting the environment variable VI_LIC_SERVER
   to the node name, e.g. in your ~/.bashrc: 'export VI_LIC_SERVER=FIXME(server)'
EOF
}

function usage() {
    cat <<EOF
Usage: install-vtd [-c] [-d DIR] [-f FILE] [-u USER] [-p PASS] [-w] <PATH>

Options:
    -h, --help              print this usage message.
    -c, --check             check whether the VTD package exists and print the md5sum.
    -d, --download-dir DIR  use DIR as the download directory for the VTD package;
                            by default, DIR = /tmp
    -f, --file FILE         use FILE as vtd installation package instead of the default;
                            this file must exist.
    -u, --user USER         username for authentication.
    -p, --pass PASS         password for authentication.
    -w, --download-only     only download the VTD package, do not install it.

Note: It is currently not recommended to install VTD system wide.
      Please use your home directory or ~/vires as the destination.
EOF
}

# Parse options:
destdir=""
downdir="/tmp"
pkgfile=""
check=false
download_only=false
let positional=0
while [[ $# -ne 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -c|--check)
            check=true
            ;;
        -d|--download-dir)
            if [[ ! -d $2 ]]; then
                echo "Error: download directory '$2' does not exist." >&2
                exit 1
            fi
            downdir=$2
            shift
            ;;
        -f|--file)
            if [[ ! -f $2 ]]; then
                echo "Error: the file '$2' does not exist." >&2
                exit 1
            fi
            pkgfile=$2
            ;;
        -p|--pass)
            vtd_pass=$2
            shift
            ;;
        -u|--user)
            vtd_user=$2
            shift
            ;;
        -w|--download-only)
            download_only=true
            ;;
        *)
            let positional++
            if [[ -d "$1" ]]; then
                destdir=$1
            else
                echo "Error: destination directory does not exist." >&2
                exit 1
            fi
            ;;
    esac
    shift
done

if [[ $check == true ]]; then
    vtd_check_url
    exit 0
fi

if [[ $positional -gt 1 ]]; then
    echo "Error: expecting only one positional argument." >&2
    usage
    exit 1
elif [[ $download_only == false && $positional -eq 0 ]]; then
    echo "Error: expecting destination path."
    usage
    exit 1
fi

# Run program:
if [[ ! -f $pkgfile ]]; then
    vtd_download "$downdir"
    pkgfile="$downdir/$vtd_pkgfile"
fi

if [[ $download_only == false ]]; then
    vtd_install "$pkgfile" "$destdir"
    vtd_configure
fi
