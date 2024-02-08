import sys
sys.path.append('/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib')
from pathlib import Path
import _cloe_bindings as cloe


databroker_adapter = cloe.DataBrokerAdapter()
driver = cloe.SimulationDriver(databroker_adapter)

stack = cloe.Stack()
conf = dict(
    version="4",
    include=[str(Path(__file__).parent / "config_minimator_smoketest.json")],
    server=dict(listen=False, listen_port=23456)
)
stack.merge(conf)


sim = cloe.Simulation(stack, driver, uuid="123")
sim.run()



# sim.run
#   -> ctx is created (with coordinator)
#       -> coordinator can be used to insert triggers (see process_pending_lua_triggers)
#       -> make_trigger(sol::table) converts sol table to triggerptr, which is what we need as well for python binding

# insert trigger on loop
#   -> test case lives 'inside' loop, inserting further triggers and blocking until they return
