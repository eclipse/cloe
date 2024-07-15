#!/bin/bash

CLOE_ROOT="$(git rev-parse --show-toplevel)"
CLOE_CONF="${CLOE_ROOT}/tests/conanfile_deployment.py"

cloe_launch() {
  PYTHONPATH="${CLOE_ROOT}/cli" python3 -m cloe_launch "$@"
}

usage_json() {
  local plugin="$1"
  shift 1
  local launch_options="$@"
  cloe_launch exec "${CLOE_CONF}" $launch_options -- -l info usage --json $plugin \
    | sed -r 's#"\$id": ".*/\.conan/data#"$id": "~/.conan/data#'
}

usage_user() {
  local plugin="$1"
  shift 1
  local launch_options="$@"
  cloe_launch exec "${CLOE_CONF}" $launch_options -- -l info usage $plugin \
    | sed -r 's#Path: .*/\.conan/data#Path: ~/.conan/data#'
}

refdir="${CLOE_ROOT}/docs/reference/plugins"
for plugin in "${refdir}"/*.rst; do
  plugin_ref="$(head -1 "$plugin" | sed -nr 's/^.. reference-gen: ?(.+)$/\1/p')"
  plugin_pkg="$(head -10 "$plugin" | sed -nr 's/^.. conan-pkg: ?(.+)$/\1/p')"
  plugin_name="$(basename "$plugin" .rst)"
  if [[ -z "${plugin_ref}" ]]; then
    echo "error: missing plugin name from: ${plugin}"
    continue
  fi
  if [[ -n "${plugin_pkg}" ]] && [[ -z "$(conan search --raw ${plugin_pkg})" ]]; then
    echo "error: required package not available in Conan cache: ${plugin_pkg}"
    continue
  fi

  launch_options=
  if [[ "$plugin_ref" == "vtd" ]]; then
     launch_options="-o with_vtd=True -o with_esmini=False"
  fi

  echo "info: generating usage for: $plugin_ref"
  usage_json ${plugin_ref} "${launch_options}"> "${refdir}/${plugin_name}_schema.json"
  usage_user ${plugin_ref} "${launch_options}"> "${refdir}/${plugin_name}.yaml"
done
