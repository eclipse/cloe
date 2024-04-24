from dataclasses import dataclass
from datetime import timedelta
from pathlib import Path
import os
from time import sleep
from typing import Optional, Dict, Any
from queue import Queue, Empty

from ._cloe_bindings import SimulationDriver, CallbackResult, DataBrokerAdapter, SimulationDriver, Stack
from ._cloe_bindings import Simulation as _Simulation


@dataclass
class TestingReportEntry:
    left: str
    op: str
    right: str
    result: str
    msg: str

    def __str__(self):
        return f"Check {self.left} {self.op} {self.right} is {self.result} (msg: {self.msg})"


class TestingReport:

    def __init__(self):
        self._entries = []

    def append_result(self, left, op, right, result, msg):
        self._entries.append(TestingReportEntry(left=left, op=op, right=right, result=result, msg=msg))

    def __str__(self):
        out = f"Testing report\n"
        out += f"--------------\n"
        for entry in self._entries:
            out += f"{entry}\n"
        return out


class TestRunner:
    def __init__(self, driver: SimulationDriver, the_test):
        self._sync = None
        self.driver = driver
        self._the_test_gen = the_test(self)
        self.report = TestingReport()

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
            return CallbackResult.Ok

        self.driver.add_trigger(self._sync, "wait_until_callback", {"name": "time", "time": seconds},
                                wait_until_callback, False)

    def wait_until(self, condition):
        def wait_for_callback(sync):
            self._sync = sync
            if condition(sync):
                try:
                    next(self._the_test_gen)
                except StopIteration:
                    ...
                return CallbackResult.Unpin
            else:
                return CallbackResult.Ok

        self.driver.add_trigger(self._sync, "wait_for_callback", {"name": "loop"}, wait_for_callback, True)

    def check_eq(self, actual, desired, err_msg=''):
        import numpy as np
        try:
            np.testing.assert_equal(actual, desired, err_msg=err_msg)
            passed = True
            msg = "success"
        except AssertionError as e:
            msg = str(e)
            passed = False
        self.report.append_result(str(actual), "==", str(desired), passed, msg)

    @property
    def signals(self):
        return self.driver.signals()

    def __call__(self, sync):
        print("available signals", self.driver.available_signals)
        self._sync = sync
        next(self._the_test_gen)
        return CallbackResult.Ok


class InteractiveRunner:
    def __init__(self, driver: SimulationDriver, simulation: _Simulation):
        self._sync = None
        self.q = Queue(maxsize=1)
        self.done = object()
        self.driver = driver
        self._sim = simulation
        self.driver.register_trigger("python_test_runner", {"name": "start"}, self, False)

        def run():
            self._sim.run()

        from threading import Thread
        t = Thread(target=run)
        t.start()
        self.q.get(True)

    def __next__(self):
        return self

    def __call__(self, sync):
        self._sync = sync
        self.q.put(sync)
        self.q.join()
        return CallbackResult.Ok

    @property
    def sync(self):
        return self._sync

    @property
    def signals(self):
        return self.driver.signals()

    def advance_by(self, time: timedelta):
        def wait_until_callback(sync):
            print("callback received!")
            self._sync = sync
            self.q.put(True, False)
            self.q.join()
            return CallbackResult.Ok

        until = self._sync.time + time
        assert until > self._sync.time
        self.driver.add_trigger(self._sync, "wait_until_callback", {"name": "time", "time": int(until.total_seconds())},
                                wait_until_callback, False)
        sleep(.1)
        self.q.task_done()
        while True:
            try:
                if self.q.get(True, timeout=.1):
                    break
            except Empty:
                pass

    def finish(self):
        self.q.task_done()


class Simulation:

    def run(self, test=None):
        if test:
            test_runner = TestRunner(self.driver, test)
            self.driver.register_trigger("python_test_runner", {"name": "start"}, test_runner, False)
        self._sim.run()

    def run_interactive(self):
        return InteractiveRunner(self.driver, self._sim)

    @property
    def log_level(self):
        return self._sim.log_level

    @log_level.setter
    def log_level(self, value):
        self._sim.log_level = value

    def bind_plugin_types(self, lib: Path):
        import importlib
        import importlib.util
        import sys
        components = str(lib.name).split('.')
        module_name = components[0]
        print(f"Attempting to load module {module_name} from {lib}")
        try:
            spec = importlib.util.spec_from_file_location(module_name, lib)
            mod = importlib.util.module_from_spec(spec)
            sys.modules[module_name] = mod
            spec.loader.exec_module(mod)
            # declare!
            mod.declare(self.databroker_adapter)
        except RuntimeError as e:
            print(f"Failed to load module {module_name}:", e)

    def __init__(self, stack: Optional[Dict[str, Any]] = None):
        self.databroker_adapter = DataBrokerAdapter()
        self.driver = SimulationDriver(self.databroker_adapter)
        plugin_paths = os.environ["CLOE_PLUGIN_PATH"].split(":")
        full_config_stack = Stack(plugin_paths)
        if not stack:
            # todo this is just here for debugging
            stack = dict(
                version="4",
                include=[str(Path(__file__).parent / "test" / "config_minimator_smoketest.json")],
                server=dict(listen=False, listen_port=23456)
            )
        full_config_stack.merge(stack)

        self._sim = _Simulation(full_config_stack, self.driver, uuid="123")

        if "BASIC_CLOE_PYTHON_BINDINGS" in os.environ:
            import importlib.util
            import sys
            binding_libs = []
            for binding_dir in os.environ["BASIC_CLOE_PYTHON_BINDINGS"].split(":"):
                if len(str(binding_dir)) > 1:
                    binding_path = Path(binding_dir)
                    if binding_path.exists():
                        binding_libs += [f.absolute() for f in binding_path.glob("*.so")]
            binding_libs = sorted(list(set(binding_libs)))  # unique and sorted
            for lib in binding_libs:
                self.bind_plugin_types(lib)
        else:
            print("no cloe python bindings environment variable defined.")
