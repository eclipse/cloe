#!/bin/bash
set -e

# This entrypoint script wraps VTD like it would be running in the foreground
# VTD's startscript actually terminates itself and the main simulator proceeses
# are regularly a bunch of orphans.
# In order to compensate for this misbehavior, this script monitor's the state
# of the most important process, the simServer. Its termination leads to the
# container's termination.
# All arguments are passed unchanged to the vtdStart.sh script.
cd /vtd_setups && cp -r Cloe* /VTD/Data/Setups
cd /VTD/Data/Setups && \
    rm -f Current && \
    ln -s Cloe.noGUInoIG Current

cd /VTD && bin/vtdStart.sh $@

# workaround for vtd start script starting vtd in the background
while pidof simServer > /dev/null
do
  sleep 1
done