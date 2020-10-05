# setup.sh
# vim: set ft=zsh:

CLOE_PLUGIN_VTD_ROOT=$(readlink -f "$(dirname ${BASH_SOURCE[0]})/..")
CLOE_ROOT=$(readlink -f "$CLOE_PLUGIN_VTD_ROOT/../..")
source "$CLOE_ROOT/tools/sh/color.sh"
source "$CLOE_ROOT/tools/sh/versions.sh"

cloe_find_vtd () {
    local quiet=${1}

    if [[ -z ${VTD_ROOT} ]]; then
        if [[ -z $quiet ]]; then
            warnf "Attempting to auto-discover VTD_ROOT."
        fi
        for vtddir in "VTD" "VTD.2.1" "VTD.2.0"; do
            for dir in "$HOME/$vtddir" \
                "$HOME/vires/$vtddir" \
                "/opt/$vtddir" \
                "/opt/vires/$vtddir" \
                "/opt/VIRES/$vtddir" \
                "/$vtddir" # used inside docker containers
            do
                if [[ -d $dir ]]; then
                    export VTD_ROOT=$dir
                    break 2
                fi
            done
        done
    fi
}

cloe_warn_vtd () {
    cloe_find_vtd
    if [[ -z ${VTD_ROOT} ]]; then
        warnf "Could not set VTD_ROOT to VTD root." >&2
    fi
}

cloe_assert_vtd () {
    cloe_find_vtd
    if [[ -z ${VTD_ROOT} ]]; then
        fatalf "Require VTD_ROOT to be set in order to continue!" >&2
    fi
}

cloe_link_projects () {
    cloe_assert_vtd
    local project=cloe_tests
    cd "${VTD_ROOT}/Data/Projects"
    if [[ -e "${project}" ]] && ! [[ -L "${project}" ]]; then
        fatalf "Existing VTD Project ${project} is not a link and can thus not be safely removed."
    elif [[ -L ${project} ]]; then
         rm -f "${project}"
    fi
    stepf "Link project: ${project}"
    ln -s "${CLOE_PLUGIN_VTD_ROOT}/contrib/projects/${project}" ${project}
    cd -
}

cloe_check_vtd_setup_path () {
    if [[ -z $vtd_setup ]]; then
        fatalf "vtd_setup not set." >&2
    fi
    local setup_path="${VTD_ROOT}/Data/Setups/${vtd_setup}"
    if ! [[ -d "${setup_path}" ]]; then
        fatalf "VTD setup does not exist: ${setup_path}" && exit 1
    fi
}

cloe_check_vtd_project_path () {
    if [[ -z $vtd_project ]]; then
        fatalf "vtd_project not set." >&2
    fi
    local project_path="${VTD_ROOT}/Data/Projects/${vtd_project}"
    if ! [[ -d "${project_path}" ]]; then
        fatalf "VTD project does not exist: ${project_path}" && exit 1
    fi
}

cloe_check_osi_vtd () {
    local exist_osi_plugin=$(find "${VTD_ROOT}/Data" -name libModuleOsi3Fmu.so)
    if [[ -n "${exist_osi_plugin}" ]]; then
        infof "Found VTD OSI plugin: ${exist_osi_plugin}" >&2
        local plugins_dir=$(dirname "${exist_osi_plugin}")
        local vtd_osi_lib="${VTD_ROOT}/Data/Setups/Cloe.noGUInoIG/Bin/libopen_simulation_interface.so"
        if ! [[ -L "${vtd_osi_lib}" && -f "${vtd_osi_lib}" ]]; then
            warnf "${vtd_osi_lib} does not exist. You may try: ./plugins/vtd/scripts/setup-vtd" >&2
        fi
        if ! [[ -L "${plugins_dir}/OSMPDummySensor.so" && -f "${plugins_dir}/OSMPDummySensor.so" ]]; then
            infof "OSI Dummy Sensor not found in ${plugins_dir}." >&2
            infof "Reinstalling.." >&2
            export LINK_TRG_PATH="${plugins_dir}"
            make -C "${CLOE_PLUGIN_VTD_ROOT}/contrib/models/osi_dummy_sensor"
        fi
    fi
}

cloe_copy_setups () {
    cloe_assert_vtd
    for setup in $CLOE_PLUGIN_VTD_ROOT/contrib/setups/*; do
        stepf "Copy setup: $setup"
        local dest="$VTD_ROOT/Data/Setups/$(basename $setup)"
        if [[ -d "$dest" ]]; then
            rm -rf "$dest"
        fi
        cp -r "$setup" "$dest"
    done
}
