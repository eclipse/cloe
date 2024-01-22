import sys
from pathlib import Path
sys.path.append('/home/ohf4fe/dev/sil/cloe/build/linux-x86_64-gcc-8/Debug/lib')
import _cloe_bindings as cloe

stack = cloe.Stack()
conf = dict(
    version="4",
    include=[str(Path(__file__).parent / "config_minimator_smoketest.json")],
    server=dict(listen=False, listen_port=23456)
)
stack.merge(conf)

sim = cloe.create_sim(stack, uuid="123")
sim.run()
