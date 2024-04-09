from datetime import timedelta
from pathlib import Path

from cloe import Simulation, TestRunner


def the_glorious_test(runner: TestRunner):
    print("bound signals", runner.signals.bound_signals())
    print("test sync time", runner.sync.time)
    yield runner.wait_duration(1)
    print("test sync time", runner.sync.time)
    acc_signal_value = runner.signals.getter("vehicles.default.basic.acc")()
    print("acc signal value (world sensor)", acc_signal_value.world_sensor)
    yield runner.wait_until(lambda sync: sync.time > timedelta(seconds=2))
    print("test sync time", runner.sync.time)


sim = Simulation(stack=dict(version="4",
                            include=[str(Path(__file__).parent / "config_minimator_smoketest.json")],
                            server=dict(listen=False, listen_port=23456)))
# sim.bind_plugin_types(Path("/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/"
#                           "Debug/lib/_basic_bindings.cpython-310-x86_64-linux-gnu.so"))
sim.log_level = "err"
sim.driver.require_signal("vehicles.default.basic.acc")
# sim.run(the_glorious_test)
run_interactive = sim.run_interactive()
print("start", run_interactive.sync.time)
acc_getter = run_interactive.signals.getter("vehicles.default.basic.acc")
run_interactive.advance_by(timedelta(seconds=1))
print("after one sec", run_interactive.sync.time)
print("acc state", acc_getter())
run_interactive.advance_by(timedelta(seconds=2))
print("after two sec", run_interactive.sync.time)
run_interactive.advance_by(timedelta(seconds=3))
print("acc state", acc_getter())
run_interactive.finish()
