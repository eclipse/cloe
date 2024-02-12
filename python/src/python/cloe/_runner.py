from pathlib import Path

import _cloe_bindings as cloe


class TestRunner:
    def __init__(self, driver: cloe.SimulationDriver, the_test):
        self._sync = None
        self.driver = driver
        self._the_test_gen = the_test(self)

    @property
    def sync(self):
        return self._sync

    def wait_until(self, seconds):
        def wait_until_callback(sync):
            self._sync = sync
            try:
                next(self._the_test_gen)
            except StopIteration:
                pass
            return cloe.CallbackResult.Ok

        self.driver.add_trigger(self._sync, "python_loop", {"name": "time", "time": seconds}, wait_until_callback, False)

    def __call__(self, sync):
        self._sync = sync
        next(self._the_test_gen)
        return cloe.CallbackResult.Ok


class SimulationContext:

    def run_simulation(self):
        self.sim.run()

    def __init__(self, test_my_stuff):
        databroker_adapter = cloe.DataBrokerAdapter()
        self.driver = cloe.SimulationDriver(databroker_adapter)
        self._test_runner = TestRunner(self.driver, test_my_stuff)

        self.driver.register_trigger("python_test_runner", {"name": "start"}, self._test_runner, False)

        stack = cloe.Stack()
        conf = dict(
            version="4",
            include=[str(Path(__file__).parent / "test" / "config_minimator_smoketest.json")],
            server=dict(listen=False, listen_port=23456)
        )
        stack.merge(conf)

        self.sim = cloe.Simulation(stack, self.driver, uuid="123")

    def __call__(self, sync):
        # print("call", sync.time)
        return cloe.CallbackResult.Ok

