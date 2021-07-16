import os

from cloe_launch.exec import PluginSetup


class SumoSetup(PluginSetup):
    name = "sumo"
    plugin = "simulator_sumo.so"

    def environment(self):
        self.env.preserve("SUMO_ROOT")

    def setup(self):
        pass

    def teardown(self):
        pass
