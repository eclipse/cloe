App
===

``App`` is the top level component of Cloe UI. It is defined in ``src/App.js``
and has the following tasks:

- Check the connection to a Cloe Host
- Fetch data from Cloe in a specified interval
- Store all data of Cloe UI
- Render all subcomponents and pass them the needed data

Check Connection To Cloe Host
-----------------------------

During ``App``'s lifecycle method ``componentDidMount``, an interval
``connectionCheck`` is registered which checks once a second for
the connection to the specified Cloe Host. If the connection state changes,
the boolean value of ``this.state.connected`` gets updated accordingly.
``this.state..connected`` is used by ``App``'s subcomponents to decide if
being rendered or not.

Fetch Data From Cloe
--------------------

The method ``renewFetchInterval`` registers an data fetch interval with a given
interval duration. The data fetch method which will get called in the interval
is ``fetchData``. Previous registered intervals will be cleared.

``renewFetchInterval`` is called in three different cases: once during the
start of Cloe UI in the ``componentDidMount`` method, every time the update
interval gets changed via the ``setUpdateInterval`` method and if a new
cloe host is specified with the ``updateHost`` method.

Store Cloe UI Data
------------------

``App`` stores all the data fetched from the Cloe API in
``this.state.cloeData``. This includes the following properties:

- ``controllers``
- ``simulation``
- ``sensors``
- ``binding``
- ``vehicles``
- ``events``
- ``simTime``

Render Subcomponents
--------------------

The ``render`` method of ``App`` creates an array ``components`` which includes
all subcomponents to render. The data for each component is passed with React
Props.

.. code-block:: jsx
    :linenos:

    const components = [
      {
        id: 0,
        component: controllers
          ? controllers.map(singleController => (
              <Controller
                key={singleController.endpointBase}
                host={host}
                controller={singleController}
                connected={connected}
                updateInterval={updateInterval}
                simTime={simTime}
              />
            ))
          : ""
      },
      { id: 1, component: <Rendering sensors={sensors} /> },
      { id: 2, component: <Simulation simulation={simulation} host={host} /> },
      { id: 3, component: <Binding binding={binding} /> },
      {
        id: 4,
        component: <VehicleList vehicles={vehicles} simTime={simTime} />
      },
      { id: 5, component: <EventListWrapper events={events} /> }
    ];

The returned JSX of ``render`` assigns the single components in the
``components`` array into two bootstrap columns. The assignment is
made with the two arrays ``this.itemsOnLeftSide`` and
``this.itemsOnRightSide`` in the constructor of ``App``.

.. code-block:: jsx
    :linenos:

    this.itemsOnLeftSide = cookies.get("firstColumn") || [0];
    this.itemsOnRightSide = cookies.get("secondColumn") || [1, 2, 3, 4, 5];



The returned JSX wraps each component in a ``DndWrapper`` component which
enables Drag&Drop functionality.
