from datetime import timedelta
from pathlib import Path

from cloe import Simulation, TestRunner

#
# def the_glorious_test(runner: TestRunner):
#     print("bound signals", runner.signals.bound_signals())
#     print("test sync time", runner.sync.time)
#     yield runner.wait_duration(1)
#     print("test sync time", runner.sync.time)
#     acc_signal_value = runner.signals.getter("vehicles.default.basic.acc")()
#     print("acc signal value (world sensor)", acc_signal_value.world_sensor)
#     yield runner.wait_until(lambda sync: sync.time > timedelta(seconds=2))
#     print("test sync time", runner.sync.time)
#
#
sim = Simulation()
sim.bind_plugin_types(Path("/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib/_basic_bindings.cpython-310-x86_64-linux-gnu.so"))
sim._sim.log_level = "err"
sim.driver.require_signal("vehicles.default.basic.acc")
# sim.run(the_glorious_test)


run_interactive = sim.run_interactive()
print("huhu", run_interactive._sync.time)
run_interactive.advance_by(timedelta(seconds=1))
print("yup", run_interactive._sync.time)
run_interactive.advance_by(timedelta(seconds=1))
print("yup2", run_interactive._sync.time)
run_interactive.finish()
