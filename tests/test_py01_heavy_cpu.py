from pathlib import Path
from random import random

from cloe import Simulation, CallbackResult

n_calls = 0

def generate_some_load(sync):
    global n_calls
    n_calls += 1

    arr = []
    for i in range(1000):
        arr.append(random())
    arr = sorted(arr)
    assert all(arr[i] <= arr[i+1] for i in range(len(arr) - 1))
    return CallbackResult.Ok


def finish(sync):
    global n_calls
    print("n_calls", n_calls)
    print("achievable realtime factor", sync.achievable_realtime_factor)
    assert n_calls == 100
    assert sync.achievable_realtime_factor > 1
    return CallbackResult.Ok


sim = Simulation(stack=dict(
    version="4",
    include=[str(Path(__file__).parent / "config_minimator_infinite.json")],
    server=dict(listen=False, listen_port=23456),
    triggers=[
        dict(event="time=2s", action="stop")
    ]
))
sim.log_level = "err"
sim.driver.register_trigger("generate_some_load", {"name": "loop"}, generate_some_load, True)
sim.driver.register_trigger("finish", {"name": "stop"}, finish, False)
sim.run()
