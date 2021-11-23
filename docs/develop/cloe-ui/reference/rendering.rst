Rendering
=========

The ``rendering`` component renders a graphical 3D scene of the simulation.
The 3D library used for that is three.js.

Props
-----

The ``rendering`` component has three props, each of them includes sensordata.

- ``egoSensor``
- ``worldSensor``
- ``laneSensor``

The data from ``egoSensor`` is used to render the egoVehicle. ``worldSensor``
is used to render world objects and ``laneSensor`` to render the street lanes.

Rendering Preparation
---------------------

The ``rendering`` component includes a method ``componentDidMount``. It
executes different preparation steps. The most important are

1. Creating three.js objects like ``scene``, ``renderer``, ``camera``
2. Set settings like camera position, background color, lighting
3. Create constants for objects (``geometry`` & ``material``)

Update Scene
------------

Each time the component receives new props, the ``render`` method updates
all objects in the scene. This is done by the method ``updateObjects``.
This method executes four steps:

1. Remove All Street Lanes
++++++++++++++++++++++++++
It is easier to remove all lanes and rerender them instead of update all lanes
correctly. This is done with the method ``removeTemporaryObjectsFromScene``,
which removes all objects from the scene which have the property
``object.temporary`` set to true.

2. Create The Ego Object
++++++++++++++++++++++++
The ego object is only created once. So there's a check if it exists already.
If not, the object is created with the method ``createEgoVehicle``.

3. Create/Update/Remove World Objects
+++++++++++++++++++++++++++++++++++++
In order to create the world objects, there's a loop through all objects given
by the ``worldSensor`` prop. It is checked with an id if there is an
corresponding object in the 3D scene already. If yes, the object will be
updated correctly with the method ``updateWorldObject``. If the object does
not exist yet, it will be created with the method ``placeWorldObject``.
After that, all objects are checked if they still exist. This is done
by checking if they have a corresponding object with the same id in
``worldSensor``. If not, the object will be removed from the scene.

4. Create Street Lanes
++++++++++++++++++++++
The last step is to add the street lanes to the scene. This is done with the
method ``updateLanes``.
