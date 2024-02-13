from pathlib import Path

import _basic_bindings
import _cloe_bindings as cloe


class TestRunner:
    def __init__(self, driver: cloe.SimulationDriver, the_test):
        self._sync = None
        self.driver = driver
        self._the_test_gen = the_test(self)

    @property
    def sync(self):
        return self._sync

    def wait_duration(self, seconds):
        def wait_until_callback(sync):
            self._sync = sync
            try:
                next(self._the_test_gen)
            except StopIteration:
                pass
            return cloe.CallbackResult.Ok

        self.driver.add_trigger(self._sync, "python_loop", {"name": "time", "time": seconds}, wait_until_callback,
                                False)

    def wait_until(self, condition):
        def wait_for_callback(sync):
            self._sync = sync
            if condition(sync):
                try:
                    next(self._the_test_gen)
                except StopIteration:
                    ...
                return cloe.CallbackResult.Unpin
            else:
                return cloe.CallbackResult.Ok

        self.driver.add_trigger(self._sync, "wait_for_python_loop", {"name": "loop"}, wait_for_callback, True)

    @property
    def signals(self):
        return self.driver.signals()

    def __call__(self, sync):
        print("available signals", self.driver.available_signals)
        self._sync = sync
        next(self._the_test_gen)
        return cloe.CallbackResult.Ok


class SimulationContext:

    def run_simulation(self):
        self.sim.run()

    def __init__(self, the_test):
        self.databroker_adapter = cloe.DataBrokerAdapter()
        _basic_bindings.declare(self.databroker_adapter)
        self.driver = cloe.SimulationDriver(self.databroker_adapter)
        self._test_runner = TestRunner(self.driver, the_test)

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
