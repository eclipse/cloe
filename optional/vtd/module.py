import os

from cloe_launch.exec import PluginSetup


class VtdSetup(PluginSetup):
    name = "vtd"
    plugin = "simulator_vtd.so"

    def environment(self):
        self.env.preserve("VTD_ROOT")
        self.env.preserve("VI_LIC_SERVER")

    def setup(self):
        pass

    def teardown(self):
        pass
