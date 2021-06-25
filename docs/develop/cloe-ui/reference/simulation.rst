Simulation
==========
The ``simulation`` component displays several information about the simulation
run and includes functions to adjust the simulation speed.

Props
-----

``simulation`` takes three props.

The ``host`` prop is necessary to trigger
the Cloe-API endpoint when adjusting the simulation speed.

The ``simulation`` prop includes the data which is displayed by the
component.

The ``host`` prop is a string with the current host address of the Cloe API.

Methods
-------

setInitialState()
+++++++++++++++++
After the component is mounted, the method ``setInitialState()`` waits for
the prop ``simulation`` and updates the components ``simSpeed`` state.
This results from the fact that the data fetching from the Cloe-Api starts
after all components have been mounted.

handleSimSpeedChange()
++++++++++++++++++++++
This method takes a value from the slider which is used to adjust the
simulation speed. First, it updates the components state to the new
``simSpeed`` value, after that it calls the method ``triggerHMI`` to
make an api call which updates the simulation speed.

triggerHMI()
++++++++++++
This method takes a destination and a value as arguments and makes an
api call to the cloe host.
