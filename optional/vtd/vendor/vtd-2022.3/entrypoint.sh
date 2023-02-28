#!/bin/bash
set -e
# This entrypoint script wraps VTD so it is as if it were running in the foreground.
#
# VTD's start script terminates itself, so that the main simulator processes
# are regularly a bunch of orphans (zombies).
# In order to compensate for this misbehavior, this script monitors the state
# of the most important process, the simServer. Its termination leads to the
# container's termination.
#
# All arguments are passed unchanged to the vtdStart.sh script.
# Make Cloe setups available to VTD, since it expects everything
# in its own directory.
cd /vtd_setups
cp -r Cloe* /VTD/Data/Setups
# Within a container, use the noGUInoIG setup, since
# we are not running in a desktop environment.
cd /VTD/Data/Setups
ln -sf Cloe.noGUInoIG Current
# Start VTD
cd /VTD
bin/vtdStart.sh $@

pid=$(pidof simServer)
wait $pid