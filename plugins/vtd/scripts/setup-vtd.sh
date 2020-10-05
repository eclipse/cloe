#!/bin/bash
set -e

source $(readlink -f "$(dirname ${BASH_SOURCE[0]})/setup.sh")

cloe_assert_vtd

infof "Install Cloe setups..."
cloe_copy_setups

infof "Link Cloe test project..."
cloe_link_projects

infof "Done."
