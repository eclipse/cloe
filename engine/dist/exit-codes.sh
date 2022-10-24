#!/bin/bash
#
# This file exports cloe-engine exit codes. This file can be used in tests
# to assert that the output of a simulation has/does not have a certain
# status. Feel free to copy out the lines or copy the file, but avoid linking
# directly to this file.

# The following exit codes are used by cloe-engine and can be used to assert
# that the outcome of the test is as expected.
#
# These values are taken from: src/simulation_context.hpp
export CLOE_EXIT_SUCCESS=0
export CLOE_EXIT_UNKNOWN=1
export CLOE_EXIT_NOSTART=4
export CLOE_EXIT_STOPPED=8
export CLOE_EXIT_FAILURE=9
export CLOE_EXIT_ABORTED=16

# This exit outcome is different for each environment: In Python it's 250 and
# in Bash it's 134, for example. Since we are running cloe-engine directly from
# BATS, we expect 134 when cloe-engine is force-killed.
#
# "Force kill" usually happens when the user presses Ctrl+C multiple times or
# the operating system kills the process directly.
export CLOE_EXIT_SYSKILL=134
