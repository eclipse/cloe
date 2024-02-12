import sys
from datetime import timedelta

sys.path.append('/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib')

from cloe import SimulationContext, TestRunner


# we can probably get rid of the yield by using a decorator
# that converts the glorious test to a generator and using "yield from"
# or take async and await
def the_glorious_test(runner: TestRunner):
    print("test sync time", runner.sync.time)
    yield runner.wait_duration(1)
    print("test sync time", runner.sync.time)
    yield runner.wait_until(lambda sync: sync.time > timedelta(seconds=2))
    print("test sync time", runner.sync.time)


ctx = SimulationContext(the_glorious_test)
ctx.sim.log_level = "warn"
# todo this should be replaced w/ a list and require_signals
ctx.driver.require_signal("vehicles.default.basic.aeb")
print(ctx.databroker_adapter.signals.bound_signals())
ctx.run_simulation()
