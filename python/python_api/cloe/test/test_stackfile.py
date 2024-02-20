import sys
from datetime import timedelta
from cloe import SimulationContext, TestRunner


def the_glorious_test(runner: TestRunner):
    print("bound signals", runner.signals.bound_signals())
    print("test sync time", runner.sync.time)
    yield runner.wait_duration(1)
    print("test sync time", runner.sync.time)
    acc_signal_value = runner.signals.getter("vehicles.default.basic.acc")()
    print("acc signal value (world sensor)", acc_signal_value.world_sensor)
    yield runner.wait_until(lambda sync: sync.time > timedelta(seconds=2))
    print("test sync time", runner.sync.time)


ctx = SimulationContext(the_glorious_test)
ctx.sim.log_level = "warn"
ctx.driver.require_signal("vehicles.default.basic.acc")
ctx.run_simulation()
