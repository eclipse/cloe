#!/usr/bin/env bats

load setup_bats

source "${CLOE_ROOT}/plugins/vtd/scripts/setup.sh"
cloe_find_vtd quiet

teardown_vtd() {
    "${CLOE_ROOT}/plugins/vtd/bin/vtd" stop
}

test_vtd_plugin_exists() {
    test ! -z ${VTD_ROOT}
    test -d ${VTD_ROOT}/bin
    test_plugin_exists vtd
}


test_vtd_osi_plugin_exists() {
    test -f ${VTD_ROOT}/Data/Distros/Distro/Plugins/ModuleManager/libModuleOsi3Fmu.so
}

teardown() {
    # It's harmless to stop vtd multiple times, so do it in case something goes wrong.
    if test_vtd_plugin_exists; then
        echo "Teardown VTD (from BATS)"
        teardown_vtd
    fi
}
