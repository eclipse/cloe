from dataclasses import dataclass
from pathlib import Path
import os
from typing import Optional, Dict, Any

from ._cloe_bindings import SimulationDriver, CallbackResult, DataBrokerAdapter, SimulationDriver, Stack, Simulation


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
                return CallbackResult.Unpin
            else:
                return CallbackResult.Ok

        self.driver.add_trigger(self._sync, "wait_for_python_loop", {"name": "loop"}, wait_for_callback, True)

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


class SimulationContext:

    def run_simulation(self):
        self.sim.run()

    def __init__(self, the_test, stack: Optional[Dict[str, Any]] = None):
        self.databroker_adapter = DataBrokerAdapter()
        self.driver = SimulationDriver(self.databroker_adapter)
        self._test_runner = TestRunner(self.driver, the_test)

        self.driver.register_trigger("python_test_runner", {"name": "start"}, self._test_runner, False)

        if "CLOE_PLUGIN_PATH" not in os.environ:
            # todo this is just here for debugging
            plugin_paths = ["/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib/cloe"]
        else:
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

        self.sim = Simulation(full_config_stack, self.driver, uuid="123")

        # todo this should be pulled from the config
        if "CLOE_PYTHON_BINDINGS" in os.environ:
            import importlib.util
            import sys
            binding_libs = []
            for binding_dir in os.environ["CLOE_PYTHON_BINDINGS"].split(":"):
                if len(str(binding_dir)) > 1:
                    binding_path = Path(binding_dir)
                    if binding_path.exists():
                        binding_libs += [f.absolute() for f in binding_path.glob("*")]
            binding_libs = sorted(list(set(binding_libs)))  # unique and sorted
            for lib in binding_libs:
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
