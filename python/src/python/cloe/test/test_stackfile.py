import sys
sys.path.append('/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib')
from pathlib import Path
import _cloe_bindings as cloe

stack = cloe.Stack()
conf = dict(
    version="4",
    include=[str(Path(__file__).parent / "config_minimator_smoketest.json")],
    server=dict(listen=False, listen_port=23456)
)
stack.merge(conf)

sim = cloe.Simulation(stack, uuid="123")
sim.run()
