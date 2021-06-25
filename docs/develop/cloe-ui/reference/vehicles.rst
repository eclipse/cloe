VehicleList and VehicleCard
===========================

VehicleList
-----------

The ``VehicleList`` component creates a ``VehicleCard`` component for each
vehicle.

Props
+++++
``VehicleList`` receives two props: ``vehicles`` and ``simTime``.
``vehicles`` is an array of all vehicles. For each of them, ``VehicleList``
returns a ``VehicleCard`` component.

VehicleCard
-----------

The ``VehicleCard`` component displays information about a vehicle, like
the speed or the steering angle. These data is shown by ``label`` components.
Additionally, ``VehicleCard`` includes a ``Chart`` component which shows the
speed of the vehicle over the simulation time.

The visibility of the chart is toggled by the method ``toggleChart()``.

Props
+++++
``VehicleCard`` receives a vehicle objects (``vehicle``) and the current
simulation time (``simTime``) as props. ``vehicle`` includes properties like
``speed``, ``steeringAngle`` and ``acceleration``.
