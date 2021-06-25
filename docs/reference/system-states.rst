System States
=============

The following image shows the various states that the internal Cloe simulation
loop can traverse.

.. graphviz:: flowcharts/system.dot
   :alt: System states and their transitions

The nominal set of transitions in a simulation will traverse all of the
following states:

Connect
   All components of the simulation are initialized and connected to any
   counterparts they may have.

Start
   The start of a simulation. Components may perform all necessary work that
   should happen for each simulation.

StepBegin
   The beginning of one simulation cycle, which usually represents something
   around 20 ms of simulated time. Various statistics concerning the entire
   cycle are reset. The webserver is denied access to internal data.

StepSimulator
   The simulator binding is stepped forward with the actuation data from the
   previous cycle, if any. The sensor information in the vehicles is then
   updated.

StepController
   Each controller binding is stepped forward, reading sensor data and writing
   actuation or otherwise.

StepEnd
   Various triggers are activated at this point, including transitions away
   from the nominal simulation loop. The webserver is allowed access to
   internal data again. Pending web read requests are processed.

Stop
   The simulation is over, because of a trigger or because the simulator
   binding indicated that the scenario is over.

Disconnect
   All components of the simulation are disconnected and destroyed.

Other states that may occur, but are not nominal, include the following:

Pause
   This state is active when the simulation is paused.

Resume
   This state is briefly entered when a paused simulation is resumed.

Reset
   The simulation can be triggered to enter into a Reset state by binding,
   controller, or trigger. If all of the components support restarting, then
   internal state is reset and the simulation loop repeats. If not, then
   the simulation is aborted.

Abort
   If any unrecoverable error occurs, the simulation is aborted.
