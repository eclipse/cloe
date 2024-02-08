# from cloe.testing import assert_xyz
import pytest

class Simulation:

    def __init__(self):
        self.triggers = []

    def loop_event(self, i):
        pass

    def wait_until(self, condition):
        ...

    def run(self):
        for i in range(50):
            self.loop_event(i)

@pytest.fixture
def simulation():
    # set up
    ...
    simulation = Simulation()

    def loop(t):
        pass

    simulation.add_trigger(loop)
    simulation.run()
    yield
    # tear down / report
    ...


def test_my_simulation(simulation: Simulation):
    simulation.wait_until(lambda sync: sync.time >= 5)  # pint may be used for units
    print("done waiting")
